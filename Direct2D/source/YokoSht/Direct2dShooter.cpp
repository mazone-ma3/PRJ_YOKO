// Direct2DShooter.cpp
#include <windows.h>
#include <d2d1.h>
#include <d2d1helper.h>
#include <dwrite.h>
#include <wincodec.h>
#include <xaudio2.h>
#include <xinput.h>
#include <list>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <cwchar>

#include <shellscalingapi.h>
#include <string>     // std::wstring
#include <shlwapi.h>  // Path系が必要なら

#define SCREEN_WIDTH  (256*2)
#define SCREEN_HEIGHT (192*2)

#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "shcore.lib") 
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")
#pragma comment(lib, "windowscodecs.lib")
#pragma comment(lib, "xaudio2.lib")
#pragma comment(lib, "xinput.lib")

template<class Interface>
inline void SafeRelease(Interface** ppInterfaceToRelease) {
    if (*ppInterfaceToRelease != NULL) {
        (*ppInterfaceToRelease)->Release();
        *ppInterfaceToRelease = NULL;
    }
}

#ifndef HINST_THISCOMPONENT
EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#define HINST_THISCOMPONENT ((HINSTANCE)&__ImageBase)
#endif

struct Bullet { float x, y; };
struct Enemy { float x, y;  int count, type; };
struct Star { float x, y, baseSpeed, speed, size; ID2D1SolidColorBrush* brush = nullptr; };

struct SoundEffect {
    BYTE* pData = nullptr;
    DWORD size = 0;
    WAVEFORMATEX wfx = {};
};

// exeと同じフォルダの絶対パスを取得する関数
std::wstring GetExeDirectory() {
    wchar_t path[MAX_PATH];
    GetModuleFileName(NULL, path, MAX_PATH);
    
    // 最後の '\' までを残してファイル名を削除
    wchar_t* lastSlash = wcsrchr(path, L'\\');
    if (lastSlash) {
        *lastSlash = L'\0';
    }
    return std::wstring(path) + L"\\";
}

std::wstring m_exeDir;

class ShooterGame {
public:
    ShooterGame();
    ~ShooterGame();

    HRESULT Initialize();
    void RunMessageLoop();

private:
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    LRESULT HandleMessage(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

    void UpdateInput();
    void StarUpdate();
    void GameUpdate();
    void OnRender();
    void CheckCollisions();
    void ResetGame();
    void ClampPlayer();

    HRESULT CreateDeviceResources();
    void DiscardDeviceResources();

    HRESULT LoadBitmapFromFile(PCWSTR uri, ID2D1Bitmap** ppBitmap);
    HRESULT LoadSound(const wchar_t* filename, SoundEffect& sound);
    void PlaySound(const SoundEffect& sound);
    void PlayBGM(const wchar_t* filename);
    void StopBGM();
    void CleanupVoices();
    void ToggleFullscreen();
	void InitStars();

    HWND m_hwnd = NULL;
    ID2D1Factory* m_pFactory = NULL;
    IDWriteFactory* m_pDWFactory = NULL;
    IDWriteTextFormat* m_pTextFormat = NULL;
    ID2D1HwndRenderTarget* m_pRenderTarget = NULL;

    ID2D1SolidColorBrush* m_pPlayerBrush = NULL;
    ID2D1SolidColorBrush* m_pEnemyBrush = NULL;
    ID2D1SolidColorBrush* m_pBulletBrush = NULL;
    ID2D1SolidColorBrush* m_pTextBrush = NULL;

    IWICImagingFactory* m_pWICFactory = NULL;
    ID2D1Bitmap* m_pPlayerBitmap = NULL;
    ID2D1Bitmap* m_pEnemyBitmap = NULL;
    ID2D1Bitmap* m_pSpriteBitmap = NULL;

    IXAudio2* m_pXAudio2 = NULL;
    IXAudio2MasteringVoice* m_pMasterVoice = NULL;
    IXAudio2SourceVoice* m_pBgmVoice = NULL;
    BYTE* m_pBgmBuffer = NULL;

    SoundEffect m_seLaser;
    SoundEffect m_seExplosion;
    std::vector<IXAudio2SourceVoice*> m_activeSounds;

    float playerX = 100.0f, playerY = SCREEN_HEIGHT / 2 - 32 + 0.0f;
    std::list<Bullet> playerBullets;
    std::list<Bullet> enemyBullets;
    std::vector<Enemy> enemies;
    std::vector<Star> stars;

    int scrollX = 0;
    int score = 0, lives = 3;
    int gameOver = 0;
    bool keys[256] = {};


    bool m_isFullscreen = false;
    RECT m_windowedRect = {};   // ウィンドウモード時の位置・サイズを保存
    bool m_f11Pressed = false;
    bool m_altEnterPressed = false;
    bool m_escPressed = false;

    // 追加
    float m_scaleX = 1.0f;
    float m_scaleY = 1.0f;
    D2D1::Matrix3x2F m_transform = D2D1::Matrix3x2F::Identity();

    ID2D1SolidColorBrush* m_starBrushes[5] = { nullptr };

    LARGE_INTEGER m_freq;
    LARGE_INTEGER m_lastTime;
    float m_deltaTime = 0.0f;
    const float TARGET_FRAME_TIME = 1.0f / 60.0f; // 60FPS目標
    float m_accumulator = 0.0f;
};


ShooterGame::ShooterGame() {
    std::srand(static_cast<unsigned>(std::time(nullptr)));
}

ShooterGame::~ShooterGame() {
    StopBGM();
    CleanupVoices();
    delete[] m_pBgmBuffer;
    DiscardDeviceResources();
    SafeRelease(&m_pDWFactory);
    SafeRelease(&m_pTextFormat);
    SafeRelease(&m_pFactory);
    SafeRelease(&m_pWICFactory);
    SafeRelease(&m_pXAudio2);
}

void ShooterGame::ToggleFullscreen() {
    if (!m_hwnd) return;

    if (!m_isFullscreen) {
        // ウィンドウモード → フルスクリーン
        GetWindowRect(m_hwnd, &m_windowedRect);

        SetWindowLongPtr(m_hwnd, GWL_STYLE, WS_POPUP | WS_VISIBLE);
        SetWindowLongPtr(m_hwnd, GWL_EXSTYLE, WS_EX_TOPMOST);

        int screenW = GetSystemMetrics(SM_CXSCREEN);
        int screenH = GetSystemMetrics(SM_CYSCREEN);

        SetWindowPos(m_hwnd, HWND_TOPMOST, 0, 0, screenW, screenH,
            SWP_FRAMECHANGED | SWP_SHOWWINDOW | SWP_NOOWNERZORDER);

        m_isFullscreen = true;
    }
    else {
        // フルスクリーン → ウィンドウモード
        SetWindowLongPtr(m_hwnd, GWL_STYLE, WS_OVERLAPPEDWINDOW | WS_VISIBLE);
        SetWindowLongPtr(m_hwnd, GWL_EXSTYLE, 0);

        SetWindowPos(m_hwnd, HWND_NOTOPMOST,
            m_windowedRect.left, m_windowedRect.top,
            m_windowedRect.right - m_windowedRect.left,
            m_windowedRect.bottom - m_windowedRect.top,
            SWP_FRAMECHANGED | SWP_SHOWWINDOW | SWP_NOOWNERZORDER);

        m_isFullscreen = false;
    }

    // RenderTargetを確実にリサイズ
    DiscardDeviceResources();
    DiscardDeviceResources();  // これで次回のOnRenderでスケールが再計算される


    // トグル後に少し間を空ける（連打防止）
    Sleep(150);   // 150ms
}

// 初期化
HRESULT ShooterGame::Initialize() {
    HRESULT hr;

    m_exeDir = GetExeDirectory();

    hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pFactory);
    if (FAILED(hr)) return hr;

    hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(&m_pDWFactory));
    if (FAILED(hr)) return hr;

    CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&m_pWICFactory));
    XAudio2Create(&m_pXAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR);
    if (m_pXAudio2) m_pXAudio2->CreateMasteringVoice(&m_pMasterVoice);

    m_pDWFactory->CreateTextFormat(L"Consolas", NULL, DWRITE_FONT_WEIGHT_NORMAL,
        DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 24.0f, L"ja-JP", &m_pTextFormat);

    LoadSound((m_exeDir + L"laser.wav").c_str(), m_seLaser);
    LoadSound((m_exeDir + L"explosion.wav").c_str(), m_seExplosion);

    WNDCLASSEX wcex = {};
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.hInstance = HINST_THISCOMPONENT;
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.lpszClassName = L"Direct2DShooter";
    RegisterClassEx(&wcex);

    m_hwnd = CreateWindow(L"Direct2DShooter", L"Direct2D 横スクロールシューティング",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
        SCREEN_WIDTH * 2, SCREEN_HEIGHT * 2,   // ← ここを2倍くらいに
        NULL, NULL, HINST_THISCOMPONENT, this);

    ShowWindow(m_hwnd, SW_SHOWNORMAL);
    UpdateWindow(m_hwnd);
//    SetTimer(m_hwnd, 1, 16, NULL);

    // 星初期化
/*    for (int i = 0; i < 80; ++i) {
        Star s;
        s.x = static_cast<float>(rand() % SCREEN_WIDTH);
        s.y = static_cast<float>(rand() % SCREEN_HEIGHT);
        s.speed = 0.5f + static_cast<float>(rand() % 100) / 30.0f;
        s.size = (s.speed > 2.0f) ? 2.0f : 1.0f;

        // 色をランダム割り当て
        int colorIndex = rand() % 5;
        s.brush = m_starBrushes[colorIndex];

        stars.push_back(s);
    }*/
//	InitStars();
//	CreateDeviceResources();

    gameOver = 1;
//    PlayBGM((m_exeDir + L"bgm.wav").c_str());
 
    QueryPerformanceFrequency(&m_freq);
    QueryPerformanceCounter(&m_lastTime);

    return S_OK;
}

LRESULT CALLBACK ShooterGame::WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    ShooterGame* pThis = nullptr;
    if (message == WM_CREATE) {
        pThis = reinterpret_cast<ShooterGame*>(((LPCREATESTRUCT)lParam)->lpCreateParams);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
    }
    else {
        pThis = reinterpret_cast<ShooterGame*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    }
    return pThis ? pThis->HandleMessage(hwnd, message, wParam, lParam) : DefWindowProc(hwnd, message, wParam, lParam);
}

LRESULT ShooterGame::HandleMessage(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_ACTIVATE:
        // ウィンドウが非アクティブになったら全キー状態をリセット
        if (LOWORD(wParam) == WA_INACTIVE) {
            memset(keys, 0, sizeof(keys));
        }
        break;


    case WM_TIMER:
//        UpdateInput();
//        if (!gameOver) GameUpdate(); else StarUpdate();

        InvalidateRect(hwnd, NULL, FALSE);
        return 0;
    case WM_PAINT:
//        OnRender();
        ValidateRect(hwnd, NULL);
        return 0;
    case WM_DESTROY:
        KillTimer(hwnd, 1);
        PostQuitMessage(0);
        return 0;


    case WM_SIZE:
        if (m_pRenderTarget) {
            UINT SCREEN_WIDTH1 = LOWORD(lParam);
            UINT SCREEN_HEIGHT1 = HIWORD(lParam);
            if (SCREEN_WIDTH1 > 0 && SCREEN_HEIGHT1 > 0) {
                m_pRenderTarget->Resize(D2D1::SizeU(SCREEN_WIDTH1, SCREEN_HEIGHT1));
            }
        }
        return 0;

    case WM_KEYDOWN:
//        UpdateInput();
        if (wParam == VK_ESCAPE) {
            PostQuitMessage(0);
            return 0;
        }
        break;

    }



    return DefWindowProc(hwnd, message, wParam, lParam);
}

void ShooterGame::UpdateInput() {
    // ウィンドウがアクティブでない場合は入力を受け付けない
    if (GetForegroundWindow() != m_hwnd) {
        memset(keys, 0, sizeof(keys));
        return;
    }


    keys[VK_LEFT] = ((GetAsyncKeyState(VK_LEFT) & 0x8000) != 0);
    keys[VK_RIGHT] = (GetAsyncKeyState(VK_RIGHT) & 0x8000) != 0;
    keys[VK_UP] = (GetAsyncKeyState(VK_UP) & 0x8000) != 0;
    keys[VK_DOWN] = (GetAsyncKeyState(VK_DOWN) & 0x8000) != 0;
    keys[VK_SPACE] = ((GetAsyncKeyState(VK_SPACE) & 0x8000) != 0) | ((GetAsyncKeyState('Z') & 0x8000) != 0);
    keys['X'] = (GetAsyncKeyState('X') & 0x8000) != 0;
    keys['R'] = (GetAsyncKeyState('R') & 0x8000) != 0;

    XINPUT_STATE state = {};
    if (XInputGetState(0, &state) == ERROR_SUCCESS) {
        auto& g = state.Gamepad;
        if ((g.wButtons & XINPUT_GAMEPAD_DPAD_LEFT) || g.sThumbLX < -10000) keys[VK_LEFT] = true;
        if ((g.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT) || g.sThumbLX > 10000) keys[VK_RIGHT] = true;
        if ((g.wButtons & XINPUT_GAMEPAD_DPAD_UP) || g.sThumbLY > 10000) keys[VK_UP] = true;
        if ((g.wButtons & XINPUT_GAMEPAD_DPAD_DOWN) || g.sThumbLY < -10000) keys[VK_DOWN] = true;
        if (g.wButtons & XINPUT_GAMEPAD_A) keys[VK_SPACE] = true;
        if (g.wButtons & XINPUT_GAMEPAD_B) keys['X'] = true;
        if (g.wButtons & XINPUT_GAMEPAD_START) keys['R'] = true;
    }

    // === フルスクリーン切り替え（改善版）===
    static bool f11PressedLast = false;
    bool f11Now = (GetAsyncKeyState(VK_F11) & 0x8000) != 0;

    if (f11Now && !f11PressedLast) {
        ToggleFullscreen();
    }
    f11PressedLast = f11Now;


    // Alt + Enter
    bool altPressed = (GetAsyncKeyState(VK_MENU) & 0x8000) != 0;
    bool enterPressed = (GetAsyncKeyState(VK_RETURN) & 0x8000) != 0;
    bool altEnterNow = altPressed && enterPressed;

    if (altEnterNow && !m_altEnterPressed) {
        ToggleFullscreen();
    }
    m_altEnterPressed = altEnterNow;

    // ESCで終了
    bool escNow = (GetAsyncKeyState(VK_ESCAPE) & 0x8000) != 0;
    if (escNow && !m_escPressed) {
        PostQuitMessage(0);
    }
    m_escPressed = escNow;

    if (gameOver == 1)
        if(!keys[VK_SPACE] && !keys['R'])
			gameOver = 2;

	if (gameOver == 2)
        if(keys[VK_SPACE] ||  keys['R']) ResetGame();
}

// ゲームリセット
void ShooterGame::ResetGame() {
    playerX = 60; playerY = 160; //SCREEN_HEIGHT / 2 - 32;
    score = 0; lives = 3; gameOver = 0;
    playerBullets.clear();
    enemyBullets.clear();
    enemies.clear();
    scrollX = 0;

/*    stars.clear();
    for (int i = 0; i < 80; ++i) {
        Star s;
        s.x = static_cast<float>(rand() % SCREEN_WIDTH);
        s.y = static_cast<float>(rand() % SCREEN_HEIGHT);
        s.speed = 0.5f + static_cast<float>(rand() % 100) / 30.0f;
        s.size = (s.speed > 2.0f) ? 2.0f : 1.0f;
        stars.push_back(s);
    }*/
//	InitStars();

    StopBGM();
    PlayBGM((m_exeDir + L"bgm.wav").c_str());
}

void ShooterGame::InitStars() {
    stars.clear();
    for (int i = 0; i < 80; ++i) {
        Star s;
        s.x = static_cast<float>(rand() % SCREEN_WIDTH);
        s.y = static_cast<float>(rand() % SCREEN_HEIGHT);
		s.baseSpeed = 0.5f + static_cast<float>(rand() % 100) / 30.0f;
		s.speed = s.baseSpeed;   // もし個別速度も欲しい場合
        s.size = (s.speed > 2.0f) ? 2.0f : 1.0f;
        s.brush = nullptr;                    // 最初はnullptrにしておく
        stars.push_back(s);
    }
}

void ShooterGame::ClampPlayer() {
    if (playerX < 10) playerX = 10;
    if (playerY < 32) playerY = 32;
    if (playerX > SCREEN_WIDTH-16) playerX = SCREEN_WIDTH-16;//720;
    if (playerY > SCREEN_HEIGHT-16) playerY = SCREEN_HEIGHT-16;//520;
}

HRESULT ShooterGame::LoadBitmapFromFile(PCWSTR uri, ID2D1Bitmap** ppBitmap) {
    if (!m_pWICFactory || !m_pRenderTarget) return E_FAIL;

    IWICBitmapDecoder* pDecoder = NULL;
    IWICBitmapFrameDecode* pSource = NULL;
    IWICFormatConverter* pConverter = NULL;
    HRESULT hr = m_pWICFactory->CreateDecoderFromFilename(uri, NULL, GENERIC_READ, WICDecodeMetadataCacheOnLoad, &pDecoder);
    if (SUCCEEDED(hr)) hr = pDecoder->GetFrame(0, &pSource);
    if (SUCCEEDED(hr)) hr = m_pWICFactory->CreateFormatConverter(&pConverter);
    if (SUCCEEDED(hr)) {
        hr = pConverter->Initialize(pSource, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.f, WICBitmapPaletteTypeMedianCut);
    }
    if (SUCCEEDED(hr)) {
        hr = m_pRenderTarget->CreateBitmapFromWicBitmap(pConverter, NULL, ppBitmap);
    }
    SafeRelease(&pDecoder);
    SafeRelease(&pSource);
    SafeRelease(&pConverter);
    return hr;
}

HRESULT ShooterGame::LoadSound(const wchar_t* filename, SoundEffect& sound) {
    HANDLE hFile = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return E_FAIL;

    DWORD bytesRead;
    char chunkId[4], format[4];
    DWORD chunkSize;

    ReadFile(hFile, chunkId, 4, &bytesRead, NULL);
    ReadFile(hFile, &chunkSize, 4, &bytesRead, NULL);
    ReadFile(hFile, format, 4, &bytesRead, NULL);

    if (strncmp(chunkId, "RIFF", 4) != 0 || strncmp(format, "WAVE", 4) != 0) {
        CloseHandle(hFile);
        return E_FAIL;
    }

    while (true) {
        char subChunkId[4];
        if (!ReadFile(hFile, subChunkId, 4, &bytesRead, NULL) || bytesRead < 4) break;
        ReadFile(hFile, &chunkSize, 4, &bytesRead, NULL);

        if (strncmp(subChunkId, "fmt ", 4) == 0) {
            DWORD readSize = min(chunkSize, sizeof(WAVEFORMATEX));
            ReadFile(hFile, &sound.wfx, readSize, &bytesRead, NULL);
            if (chunkSize > readSize) SetFilePointer(hFile, chunkSize - readSize, NULL, FILE_CURRENT);
        }
        else if (strncmp(subChunkId, "data", 4) == 0) {
            sound.size = chunkSize;
            sound.pData = new BYTE[chunkSize];
            ReadFile(hFile, sound.pData, chunkSize, &bytesRead, NULL);
            break;
        }
        else {
            SetFilePointer(hFile, chunkSize, NULL, FILE_CURRENT);
        }
    }
    CloseHandle(hFile);
    return S_OK;
}

void ShooterGame::PlaySound(const SoundEffect& sound) {
    if (!sound.pData || !m_pXAudio2) return;
    IXAudio2SourceVoice* pVoice = nullptr;
    if (SUCCEEDED(m_pXAudio2->CreateSourceVoice(&pVoice, &sound.wfx))) {
        XAUDIO2_BUFFER buffer = {};
        buffer.AudioBytes = sound.size;
        buffer.pAudioData = sound.pData;
        buffer.Flags = XAUDIO2_END_OF_STREAM;
        pVoice->SubmitSourceBuffer(&buffer);
        pVoice->Start(0);
        m_activeSounds.push_back(pVoice);
    }
}

void ShooterGame::CleanupVoices() {
    for (auto it = m_activeSounds.begin(); it != m_activeSounds.end(); ) {
        XAUDIO2_VOICE_STATE state;
        (*it)->GetState(&state);
        if (state.BuffersQueued == 0) {
            (*it)->DestroyVoice();
            it = m_activeSounds.erase(it);
        }
        else {
            ++it;
        }
    }
}

void ShooterGame::PlayBGM(const wchar_t* filename) {
    if (!m_pXAudio2) return;

    StopBGM();  // 念のため一旦停止

    HANDLE hFile = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return;

    // --- WAV解析（LoadSoundとほぼ同じ）---
    DWORD bytesRead;
    char chunkId[4], format[4];
    DWORD chunkSize;

    ReadFile(hFile, chunkId, 4, &bytesRead, NULL);
    ReadFile(hFile, &chunkSize, 4, &bytesRead, NULL);
    ReadFile(hFile, format, 4, &bytesRead, NULL);

    if (strncmp(chunkId, "RIFF", 4) != 0 || strncmp(format, "WAVE", 4) != 0) {
        CloseHandle(hFile); return;
    }

    WAVEFORMATEX wfx = {};
    while (true) {
        char subId[4];
        if (!ReadFile(hFile, subId, 4, &bytesRead, NULL) || bytesRead < 4) break;
        ReadFile(hFile, &chunkSize, 4, &bytesRead, NULL);

        if (strncmp(subId, "fmt ", 4) == 0) {
            DWORD readSize = min(chunkSize, sizeof(WAVEFORMATEX));
            ReadFile(hFile, &wfx, readSize, &bytesRead, NULL);
            if (chunkSize > readSize)
                SetFilePointer(hFile, chunkSize - readSize, NULL, FILE_CURRENT);
        }
        else if (strncmp(subId, "data", 4) == 0) {
            m_pBgmBuffer = new BYTE[chunkSize];
            ReadFile(hFile, m_pBgmBuffer, chunkSize, &bytesRead, NULL);
            break;
        }
        else {
            SetFilePointer(hFile, chunkSize, NULL, FILE_CURRENT);
        }
    }
    CloseHandle(hFile);

    if (m_pBgmBuffer && SUCCEEDED(m_pXAudio2->CreateSourceVoice(&m_pBgmVoice, &wfx))) {
        XAUDIO2_BUFFER buffer = {};
        buffer.AudioBytes = chunkSize;  // 実際のサイズ
        buffer.pAudioData = m_pBgmBuffer;
        buffer.Flags = XAUDIO2_END_OF_STREAM;
        buffer.LoopCount = XAUDIO2_LOOP_INFINITE;   // 無限ループ

        m_pBgmVoice->SubmitSourceBuffer(&buffer);
        m_pBgmVoice->Start(0);
    }
}

void ShooterGame::StopBGM() {
    if (m_pBgmVoice) {
        m_pBgmVoice->Stop(0);
        m_pBgmVoice->DestroyVoice();
        m_pBgmVoice = NULL;
    }
    delete[] m_pBgmBuffer;
    m_pBgmBuffer = NULL;
}

HRESULT ShooterGame::CreateDeviceResources() {
    if (m_pRenderTarget) return S_OK;

    RECT rc;
    GetClientRect(m_hwnd, &rc);
    UINT clientW = rc.right;
    UINT clientH = rc.bottom;

    if (clientW == 0 || clientH == 0) return E_FAIL;

    // DPIを正しく取得して補正
    float dpi = static_cast<float>(GetDpiForWindow(m_hwnd));
    float effectiveScale = dpi / 96.0f;

    D2D1_RENDER_TARGET_PROPERTIES props = D2D1::RenderTargetProperties(
        D2D1_RENDER_TARGET_TYPE_DEFAULT,
        D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED),
        dpi, dpi);

    D2D1_SIZE_U size = D2D1::SizeU(clientW, clientH);

    HRESULT hr = m_pFactory->CreateHwndRenderTarget(
        props,
        D2D1::HwndRenderTargetProperties(m_hwnd, size),
        &m_pRenderTarget);

    if (SUCCEEDED(hr)) {
        // 仮想解像度に対するスケール（DPI補正込み）
        m_scaleX = static_cast<float>(clientW) / (SCREEN_WIDTH * effectiveScale);
        m_scaleY = static_cast<float>(clientH) / (SCREEN_HEIGHT * effectiveScale);

        m_transform = D2D1::Matrix3x2F::Scale(m_scaleX, m_scaleY);

        // ブラシ作成
        m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0.0f, 0.8f, 1.0f), &m_pPlayerBrush);
        m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(1.0f, 0.3f, 0.3f), &m_pEnemyBrush);
        m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(1.0f, 0.9f, 0.2f), &m_pBulletBrush);
        m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(1.0f, 1.0f, 1.0f), &m_pTextBrush);

        if (!m_pPlayerBitmap) LoadBitmapFromFile((m_exeDir + L"player.png").c_str(), &m_pPlayerBitmap);


// LoadBitmapFromFile(L"player.png", &m_pPlayerBitmap);
        if (!m_pEnemyBitmap) LoadBitmapFromFile((m_exeDir + L"enemy.png").c_str(), &m_pEnemyBitmap);

//LoadBitmapFromFile(L"enemy.png", &m_pEnemyBitmap);

        if (!m_pSpriteBitmap) LoadBitmapFromFile((m_exeDir + L"yokosht.png").c_str(), &m_pSpriteBitmap);

    }
    InitStars();
    if (SUCCEEDED(hr)) {
        // 星用の色ブラシを複数作成
        m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0.9f, 0.9f, 1.0f), &m_starBrushes[0]); // 白
        m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0.8f, 0.9f, 1.0f), &m_starBrushes[1]); // 薄青
        m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(1.0f, 0.95f, 0.7f), &m_starBrushes[2]); // 薄黄
        m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0.7f, 0.8f, 1.0f), &m_starBrushes[3]); // 青白
        m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(1.0f, 0.8f, 0.8f), &m_starBrushes[4]); // 薄赤

/*        if (!stars.empty()) {
            for (auto& s : stars) {
                if (s.brush == nullptr) {
                    s.brush = m_starBrushes[rand() % 5];
                }
            }
        }*/
      for (auto& s : stars) {
            if (s.brush == nullptr || s.brush == (ID2D1SolidColorBrush*)0xcccccccc || s.brush == (ID2D1SolidColorBrush*)0xdeadbeef) {
                s.brush = m_starBrushes[rand() % 5];
            }
        }
    }
    return hr;
}

void ShooterGame::DiscardDeviceResources() {
    SafeRelease(&m_pRenderTarget);
    SafeRelease(&m_pPlayerBrush);
    SafeRelease(&m_pEnemyBrush);
    SafeRelease(&m_pBulletBrush);
    SafeRelease(&m_pTextBrush);
    SafeRelease(&m_pPlayerBitmap);
    SafeRelease(&m_pEnemyBitmap);
    SafeRelease(&m_pSpriteBitmap);

    for (int i = 0; i < 5; ++i) {
        SafeRelease(&m_starBrushes[i]);
    }
    // スケールもリセット
    m_scaleX = 1.0f;
    m_scaleY = 1.0f;
    m_transform = D2D1::Matrix3x2F::Identity();

    // 星のブラシ参照もクリア（重要！）
    for (auto& s : stars) {
        s.brush = nullptr;
    }
}

// ゲーム表示
void ShooterGame::OnRender() {
    // 必ず最初にリソース作成を試みる
    if (FAILED(CreateDeviceResources()) || !m_pRenderTarget) return;

    // BeginDrawはvoidなので、HRESULTで受け取らない
    m_pRenderTarget->BeginDraw();
    m_pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::Black));

    // スケール適用
    m_pRenderTarget->SetTransform(m_transform);

    // 星背景（ブラシが有効なときだけ）
/*    if (m_pTextBrush) {
       for (const auto& s : stars) {
            D2D1_RECT_F rect = D2D1::RectF(s.x, s.y, s.x + s.size, s.y + s.size);
            m_pRenderTarget->FillRectangle(rect, m_pTextBrush);
        }
    }*/

    if (m_pTextBrush) {
        for (const auto& s : stars) {
            D2D1_RECT_F rect = D2D1::RectF(s.x, s.y, s.x + s.size, s.y + s.size);

            if (s.brush) {
                m_pRenderTarget->FillRectangle(rect, s.brush);
            }
            else if (m_pTextBrush) {
                m_pRenderTarget->FillRectangle(rect, m_pTextBrush);  // フォールバック
            }

        }
    }
/*    for (const auto& s : stars) {
        D2D1_RECT_F rect = D2D1::RectF(s.x, s.y, s.x + s.size, s.y + s.size);

        if (s.brush) {
            m_pRenderTarget->FillRectangle(rect, s.brush);
        } else if (m_pTextBrush) {
            m_pRenderTarget->FillRectangle(rect, m_pTextBrush);
        }
    }*/

    // 自機
    if (m_pSpriteBitmap) {
        D2D1_RECT_F rect = D2D1::RectF(playerX, playerY, playerX + 31, playerY + 31);
        D2D1_RECT_F sourceRect = D2D1::RectF(32 * 1, 0, 32 * 1 + 31, 31);
        m_pRenderTarget->DrawBitmap(m_pSpriteBitmap, rect, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR, sourceRect);
    }

    // 敵
    if (m_pEnemyBrush || m_pSpriteBitmap) {
        for (const auto& e : enemies) {
            D2D1_RECT_F rect = D2D1::RectF(e.x, e.y, e.x + 31, e.y + 31);
            D2D1_RECT_F sourceRect = D2D1::RectF(32 * 2, 0, 32 * 2 + 31, 31);
            if (m_pSpriteBitmap) {
                m_pRenderTarget->DrawBitmap(m_pSpriteBitmap, rect, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR, sourceRect);
            }
            else if (m_pEnemyBrush) {
                m_pRenderTarget->FillRectangle(rect, m_pEnemyBrush);
            }
        }
    }

    // 弾
//    if (m_pBulletBrush) {
    if (m_pSpriteBitmap) {
        for (const auto& b : playerBullets) {
            //        m_pRenderTarget->FillRectangle(D2D1::RectF(b.x, b.y - 3, b.x + 8, b.y + 3), m_pBulletBrush);
            D2D1_RECT_F rect = D2D1::RectF(b.x, b.y, b.x + 31, b.y + 31);
            D2D1_RECT_F sourceRect = D2D1::RectF(32 * 4, 0, 32 * 4 + 31, 31);
            m_pRenderTarget->DrawBitmap(m_pSpriteBitmap, rect, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR, sourceRect);
        }

        for (const auto& b : enemyBullets) {
//            m_pRenderTarget->FillRectangle(D2D1::RectF(b.x, b.y - 3, b.x + 6, b.y + 3), m_pBulletBrush);
            D2D1_RECT_F rect = D2D1::RectF(b.x, b.y, b.x + 31, b.y + 31);
            D2D1_RECT_F sourceRect = D2D1::RectF(32 * 0, 0, 32 * 0 + 31, 31);
            m_pRenderTarget->DrawBitmap(m_pSpriteBitmap, rect, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR, sourceRect);
        }
    }


    // UI
    if (m_pTextFormat && m_pTextBrush) {
        wchar_t text[128];

//        swprintf_s(text, L"SCORE: %d", score);
//        m_pRenderTarget->DrawText(text, wcslen(text), m_pTextFormat,
//            D2D1::RectF(10, 10, 400, 60), m_pTextBrush);

	    swprintf_s(text, L"SCORE: %d", score);
	    m_pRenderTarget->DrawText(text, wcslen(text), m_pTextFormat, 
			D2D1::RectF(0, 0, 160, 16), m_pTextBrush);

//        swprintf_s(text, L"LIVES: %d", lives);
//        m_pRenderTarget->DrawText(text, wcslen(text), m_pTextFormat,
//            D2D1::RectF(10, 50, 400, 100), m_pTextBrush);

	    swprintf_s(text, L"LIVES: %d", lives);
	    m_pRenderTarget->DrawText(text, wcslen(text), m_pTextFormat,
	        D2D1::RectF(0, 16, 160, 32), m_pTextBrush);

    }

    if (gameOver && m_pTextFormat && m_pEnemyBrush) {
//        m_pRenderTarget->DrawText(L"GAME OVER", 9, m_pTextFormat,
//            D2D1::RectF(120, 140, 420, 200), m_pEnemyBrush);

        m_pRenderTarget->DrawText(L"GAME OVER", 9, m_pTextFormat, 
			D2D1::RectF(250/2, 220/2, 250/2+9*16, 250/2+16), m_pEnemyBrush);

        if (m_pTextBrush) {
//            m_pRenderTarget->DrawText(L"Press R to Restart", 18, m_pTextFormat,
//                D2D1::RectF(100, 200, 450, 260), m_pTextBrush);
	        m_pRenderTarget->DrawText(L"Press R to Restart", 18, m_pTextFormat, 				D2D1::RectF(250/2, 280/2, 250/2+16*18, 280/2+16), m_pTextBrush);

        }
    }


    HRESULT hr = m_pRenderTarget->EndDraw();
    if (hr == D2DERR_RECREATE_TARGET){ //} || hr == D2DERR_DEVICE_LOST) {
        DiscardDeviceResources();
    }

}

void ShooterGame::StarUpdate() {
    // 星移動
    for (auto& s : stars) {
//        s.x -= s.speed * m_deltaTime * 60.0f;   // 60.0fは基準FPS（調整用）
		s.x -= s.baseSpeed * m_deltaTime * 60.0f;   // これでFPSが60の時に元の速度と同じになる
        if (s.x < 0) {
            s.x = SCREEN_WIDTH; //800;
            s.y = static_cast<float>(rand() % SCREEN_HEIGHT); //600);
        }
    }
}

// ゲーム進行
void ShooterGame::GameUpdate() {
    StarUpdate();

	float moveSpeed = 4.0f * 60 * m_deltaTime;
	float enemySpeed = 4.0f * 60 * m_deltaTime;
	float enemySpeed2 = 5.0f * 60 * m_deltaTime;

    if (keys[VK_LEFT])  playerX -= moveSpeed;
    if (keys[VK_RIGHT]) playerX += moveSpeed;
    if (keys[VK_UP])    playerY -= moveSpeed;
    if (keys[VK_DOWN])  playerY += moveSpeed;
    ClampPlayer();

/*    static int shootCool = 0;
    if (keys[VK_SPACE] && shootCool <= 0) {
        playerBullets.push_back({ playerX + 45, playerY + 18 });
        shootCool = 5;
        PlaySound(m_seLaser);
    }
    if (shootCool > 0) shootCool--;
*/

// 射撃クールタイムも時間ベースに
    static float shootTimer = 0.0f;
    shootTimer += m_deltaTime;
    if (keys[VK_SPACE] && shootTimer >= 0.08f) {   // 約毎秒12.5発
        playerBullets.push_back({ playerX + 45, playerY + 18 });
        shootTimer = 0.0f;
        PlaySound(m_seLaser);
    }

    CleanupVoices();

    // 敵生成
    if (rand() % 32 == 0)
        enemies.push_back({ SCREEN_WIDTH+0.0f, 32.0f + (rand() % (SCREEN_HEIGHT-32-32-32)), 0, rand() % 3});

    // 敵移動 + 敵弾
    for (auto it = enemies.begin(); it != enemies.end(); ) {

        it->count += 1;

        if (it->type == 0)		// 通常敵
            it->x -= enemySpeed;
        else
            it->x -= enemySpeed2;

        if (rand() % 50 == 0) enemyBullets.push_back({ it->x, it->y + 16 });
        if (it->x < -50) it = enemies.erase(it);
        else ++it;
    }

    // 弾移動
    for (auto it = playerBullets.begin(); it != playerBullets.end(); ) {
        it->x += 13.0f * 60 * m_deltaTime;
        if (it->x > SCREEN_WIDTH) it = playerBullets.erase(it);
        else ++it;
    }
    for (auto it = enemyBullets.begin(); it != enemyBullets.end(); ) {
        it->x -= 9.0f * 60 * m_deltaTime;
        if (it->x < -30) it = enemyBullets.erase(it);
        else ++it;
    }

    CheckCollisions();
}

void ShooterGame::CheckCollisions() {
    // 自弾 vs 敵
    for (auto bit = playerBullets.begin(); bit != playerBullets.end(); ) {
        bool hit = false;
        for (auto eit = enemies.begin(); eit != enemies.end(); ) {
            if (bit->x + 18 > eit->x && bit->x < eit->x + 32 &&
                bit->y > eit->y - 5 && bit->y < eit->y + 37) {
                eit = enemies.erase(eit);
                hit = true;
                score += 10;
                PlaySound(m_seExplosion);
                break;
            }
            else ++eit;
        }
        if (hit) bit = playerBullets.erase(bit);
        else ++bit;
    }

    // 敵弾 vs 自機
    for (auto it = enemyBullets.begin(); it != enemyBullets.end(); ) {
        if (it->x + 14 > playerX && it->x < playerX + 40 &&
            it->y > playerY && it->y < playerY + 35) {
            lives--;
            it = enemyBullets.erase(it);
            if (lives <= 0) {
                gameOver = 1;
                StopBGM();
            }
        }
        else ++it;
    }

    // 敵 vs 自機
    for (auto it = enemies.begin(); it != enemies.end(); ) {
        if (it->x + 32 > playerX && it->x < playerX + 40 &&
            it->y + 32 > playerY && it->y < playerY + 35) {
            lives = 0;
            gameOver = 1;
            StopBGM();
            return;
        }
        else ++it;
    }
}

/*void ShooterGame::RunMessageLoop() {
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}*/

/*void ShooterGame::RunMessageLoop() {
    MSG msg = {};
    while (msg.message != WM_QUIT) {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else {
            // 時間計測
            LARGE_INTEGER now;
            QueryPerformanceCounter(&now);
            m_deltaTime = static_cast<float>(now.QuadPart - m_lastTime.QuadPart) / m_freq.QuadPart;
            m_lastTime = now;
            UpdateInput();
            if(!gameOver)
                GameUpdate();     // deltaTimeを使って更新
            else StarUpdate();
            InvalidateRect(m_hwnd, NULL, FALSE);
        }
    }
}*/

void ShooterGame::RunMessageLoop() {
    MSG msg = {};
    LARGE_INTEGER freq, last, now;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&last);

    while (msg.message != WM_QUIT) {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else {
            QueryPerformanceCounter(&now);
 //           float delta = static_cast<float>(now.QuadPart - last.QuadPart) / freq.QuadPart;
            m_deltaTime = static_cast<float>(now.QuadPart - last.QuadPart) / freq.QuadPart;
            last = now;

            m_accumulator += m_deltaTime; //delta;
//            UpdateInput();

            while (m_accumulator >= TARGET_FRAME_TIME) {
                UpdateInput();
                if (!gameOver)
                    GameUpdate();     // deltaTimeを使って更新
                else StarUpdate();
                m_accumulator -= TARGET_FRAME_TIME;
            }

            OnRender();           // 描画は毎回呼ぶ（垂直同期に任せる）
        }
    }
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    // DPI Awareを最優先で設定
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

    CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

    ShooterGame game;
    if (SUCCEEDED(game.Initialize())) {
        game.RunMessageLoop();
    }
    CoUninitialize();
    return 0;
}