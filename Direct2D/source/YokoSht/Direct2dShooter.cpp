// Direct2DShooter.cpp
#include <windows.h>
#include <d2d1.h>
#include <d2d1helper.h>
#include <dwrite.h>
#include <wincodec.h>
#include <xaudio2.h>
#include <xinput.h>
//#include <list>
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

// 構造体
struct Bullet { float x, y; };
struct Enemy { float x, y, shootTimer, nextShootTime; int type; float count, count2; int count_hp; bool count_flag; };
struct eBullets { float x, y, vx=0.0f, vy=0.0f; };
struct Particle { float x, y, vx, vy; int life; };

struct Option{ float offset_y, x, y; };
struct Item { float x, y; int timer, type; };

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

	void CalculateFPS();

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
//    ID2D1Bitmap* m_pPlayerBitmap = NULL;
//    ID2D1Bitmap* m_pEnemyBitmap = NULL;
    ID2D1Bitmap* m_pSpriteBitmap = NULL;

    IXAudio2* m_pXAudio2 = NULL;
    IXAudio2MasteringVoice* m_pMasterVoice = NULL;
    IXAudio2SourceVoice* m_pBgmVoice = NULL;
    BYTE* m_pBgmBuffer = NULL;

    SoundEffect m_seLaser;
    SoundEffect m_seExplosion;
    std::vector<IXAudio2SourceVoice*> m_activeSounds;

    int scrollX = 0;
    bool keys[256] = {};

    bool m_isFullscreen = false;
    RECT m_windowedRect = {};   // ウィンドウモード時の位置・サイズを保存
    bool m_f11Pressed = false;
    bool m_altEnterPressed = false;
    bool m_escPressed = false;

    float m_scaleX = 1.0f;
    float m_scaleY = 1.0f;
    D2D1::Matrix3x2F m_transform = D2D1::Matrix3x2F::Identity();

    ID2D1SolidColorBrush* m_starBrushes[5] = { nullptr };

    LARGE_INTEGER m_freq;
    LARGE_INTEGER m_lastTime;
    float m_deltaTime = 0.0f;
    const float TARGET_FRAME_TIME = 1.0f / 60.0f; // 60FPS目標
    float m_accumulator = 0.0f;

    // FPS測定用
/*    LARGE_INTEGER m_lastFPSTime;
    int m_frameCount = 0;
    float m_currentFPS = 0.0f;
    std::vector<float> m_fpsHistory;   // 移動平均用
    const int FPS_HISTORY_MAX = 30;    // 直近30フレームの平均*/

    LARGE_INTEGER m_lastUpdateTime;
    LARGE_INTEGER m_lastRenderTime;
    int m_updateFrameCount = 0;
    int m_renderFrameCount = 0;
    float m_updateFPS = 0.0f;
    float m_renderFPS = 0.0f;

    float m_gameTime = 0.0f;           // 経過時間（秒）
    float m_enemySpawnRate = 1.0f;     // 現在の敵生成頻度（小さいほど頻繁）

	// 各種変数
    float playerX = 60.0f, playerY = 160.0f;
    std::vector<Bullet> playerBullets;
    std::vector<eBullets> enemyBullets;
    std::vector<Particle> Particles;
    std::vector<Option> Options;
    std::vector<Item> Items;

    std::vector<Enemy> enemies;
    std::vector<Star> stars;

	int bomb_stok = 0;
	bool shield_active = false;

	int chain_count = 0;

	int chain_timer = 0;
	int option_cooldown = 10;
	int enemy_spawn_timer = 0;
	int kill_count = 0;
	int shoot_timer = 0;

	int score = 0, lives = 3, highscore=5000;
	int gameOver = 0;

	int play_time = 0;		  // 経過時間（フレーム）


	int sx,sy,dx,dy,ex,ey,ph_x,ph_y,ph_w,ph_h;

	int enemy_bullet_speed;

	int shoot_interval;
	int dist;
	int direction_factor;
	int offset;
};

int sin_table[256 * 4];


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
/*    QueryPerformanceCounter(&m_lastTime);

    QueryPerformanceCounter(&m_lastFPSTime);

    m_fpsHistory.reserve(FPS_HISTORY_MAX);*/
    QueryPerformanceCounter(&m_lastUpdateTime);
    QueryPerformanceCounter(&m_lastRenderTime);

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
/*
    if (gameOver == 1)
        if(!keys[VK_SPACE] && !keys['R'])
			gameOver = 2;

	if (gameOver == 2)
        if(keys[VK_SPACE] ||  keys['R']) ResetGame();*/
}

// ゲームリセット
void ShooterGame::ResetGame() {
    playerX = 60.f; playerY = 160.0f; //SCREEN_HEIGHT / 2 - 32;
    score = 0; lives = 3; gameOver = 0;
    playerBullets.clear();
    enemyBullets.clear();
    enemies.clear();
    scrollX = 0;
    m_gameTime = 0;

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
    if (playerX < 0) playerX = 0;
    if (playerY < 0) playerY = 0;
    if (playerX > SCREEN_WIDTH-40) playerX = SCREEN_WIDTH-40;//720;
    if (playerY > SCREEN_HEIGHT-32) playerY = SCREEN_HEIGHT-32;//520;
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

//        if (!m_pPlayerBitmap) LoadBitmapFromFile((m_exeDir + L"player.png").c_str(), &m_pPlayerBitmap);


//        if (!m_pEnemyBitmap) LoadBitmapFromFile((m_exeDir + L"enemy.png").c_str(), &m_pEnemyBitmap);


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
//    SafeRelease(&m_pPlayerBitmap);
//    SafeRelease(&m_pEnemyBitmap);
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

    // 描画FPS計算
    static LARGE_INTEGER lastRender = {};
    if (lastRender.QuadPart == 0) QueryPerformanceCounter(&lastRender);

    m_renderFrameCount++;

    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    float elapsed = static_cast<float>(now.QuadPart - lastRender.QuadPart) / m_freq.QuadPart;

    if (elapsed >= 1.0f) {
        m_renderFPS = m_renderFrameCount / elapsed;
        m_renderFrameCount = 0;
        lastRender = now;
    }

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
    // FPS表示（右上に2段で表示）
    if (m_pTextFormat && m_pTextBrush) {
        wchar_t text[64];

        swprintf_s(text, L"Update: %.1f", m_updateFPS);
        m_pRenderTarget->DrawText(text, wcslen(text), m_pTextFormat,
            D2D1::RectF(SCREEN_WIDTH - 180, 8, SCREEN_WIDTH, 40), m_pTextBrush);

        swprintf_s(text, L"Render: %.1f", m_renderFPS);
        m_pRenderTarget->DrawText(text, wcslen(text), m_pTextFormat,
            D2D1::RectF(SCREEN_WIDTH - 180, 35, SCREEN_WIDTH, 70), m_pTextBrush);
    }
    if (m_pTextFormat && m_pTextBrush) {
        wchar_t text[128];
/*        wchar_t fpsText[32];

        swprintf_s(fpsText, L"FPS: %.1f", m_currentFPS);
        m_pRenderTarget->DrawText(fpsText, wcslen(fpsText), m_pTextFormat,
        D2D1::RectF(SCREEN_WIDTH - 150, 10, SCREEN_WIDTH - 10, 50), m_pTextBrush);*/

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

        // TIME（LIVESの下）
        swprintf_s(text, L"TIME: %.0f", m_gameTime);
        m_pRenderTarget->DrawText(text, wcslen(text), m_pTextFormat,
            D2D1::RectF(10, 80, 300, 110), m_pTextBrush);
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
//    CalculateFPS();     // ← ここを追加

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
//    CalculateFPS();     // ← ここを追加
    m_updateFrameCount++;

    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    float elapsed = static_cast<float>(now.QuadPart - m_lastUpdateTime.QuadPart) / m_freq.QuadPart;

    if (elapsed >= 1.0f) {
        m_updateFPS = m_updateFrameCount / elapsed;
        m_updateFrameCount = 0;
        m_lastUpdateTime = now;
    }
}

// ゲーム進行
void ShooterGame::GameUpdate() {
    if (gameOver == 1)
        if(!keys[VK_SPACE] && !keys['R'])
			gameOver = 2;

	if (gameOver == 2)
        if(keys[VK_SPACE] ||  keys['R']) ResetGame();

    StarUpdate();
    if (gameOver)
		return;

/*    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    m_deltaTime = static_cast<float>(now.QuadPart - m_lastTime.QuadPart) / m_freq.QuadPart;
    m_lastTime = now;*/

    m_gameTime += m_deltaTime;

    // 敵生成頻度を時間経過で徐々に上げる
//    float progress = m_gameTime / 180.0f;           // 0.0 ~ 1.0（180秒で最大）
//    if (progress > 1.0f) progress = 1.0f;

    // 最初は敵が少なく、徐々に増える（0.8秒間隔 → 0.25秒間隔くらい）
//    m_enemySpawnRate = 0.8f - (0.55f * progress);

	float moveSpeed = 4.0f * 60 * m_deltaTime;
	float enemySpeed = 4.0f * 60 * m_deltaTime;
	float enemySpeed2 = 5.0f * 60 * m_deltaTime;

    if (keys[VK_LEFT])  playerX -= moveSpeed;
    if (keys[VK_RIGHT]) playerX += moveSpeed;
    if (keys[VK_UP])    playerY -= moveSpeed;
    if (keys[VK_DOWN])  playerY += moveSpeed;
    ClampPlayer();

// 射撃クールタイムも時間ベースに
    static float shootTimer = 0.0f;
    shootTimer += m_deltaTime;
    if (keys[VK_SPACE] && shootTimer >= 0.08f*2) {   // 約毎秒12.5/2発
        playerBullets.push_back({ playerX + 32, playerY + 12 });
        shootTimer = 0.0f;
//        PlaySound(m_seLaser);
    }

//    CleanupVoices();
	static float voiceCleanTimer = 0.0f;
	voiceCleanTimer += m_deltaTime;
	if (voiceCleanTimer > 0.5f) {     // 0.5秒ごとに掃除
	    CleanupVoices();
	    voiceCleanTimer = 0.0f;
	}
    // 敵生成
//    if (rand() % 32 == 0)
// 敵生成（時間進行で増える）
//    static float spawnTimer = 0.0f;
//    spawnTimer += m_deltaTime;
 // === 敵生成処理（スコアベース）===
    static float enemySpawnTimer = 0.0f;
    enemySpawnTimer += m_deltaTime;

    // 元のロジックをdeltaTimeに変換
    float baseInterval = 50.0f - (score / 250.0f);   // scoreが増えるほど短く
    float spawnInterval = max(0.3f, baseInterval / 60.0f);  // フレーム→秒に変換

//    if (spawnTimer >= m_enemySpawnRate) {
    if (enemySpawnTimer >= spawnInterval) {

//        struct Enemy { float x, y, shootTimer, nextShootTime; int type, count, count2, count_hp; bool count_flag; };

//        struct Enemy { float x, y, shootTimer, nextShootTime; int type; float count, count2; int count_hp; bool count_flag; };

        int type = rand() % 3;
        float count = rand() % (30 * 2 + SCREEN_HEIGHT - 40 * 2) - 30 * 2;
        enemies.push_back({ SCREEN_WIDTH+0.0f, 32.0f + (rand() % (SCREEN_HEIGHT-32-32-32)), 0.0f, 5.0f/60, type, 0.0f, count,
            (type == 0)? 1:3, false});
//        spawnTimer = 0.0f;
		enemySpawnTimer = 0.0f;
    }

    // 敵移動 + 敵弾発射
/*    for (auto it = enemies.begin(); it != enemies.end(); ) {

        it->count += 1;

        if (it->type == 0)		// 通常敵
            it->x -= enemySpeed;
        else
            it->x -= enemySpeed2;

        if (rand() % 50 == 0) enemyBullets.push_back({ it->x, it->y + 16 });
        if (it->x < -50) it = enemies.erase(it);
        else ++it;
    }*/

    for (auto& e : enemies) {
//        e.x -= /*速度*/ * m_deltaTime * 60.0f;
        e.count += m_deltaTime * 60.0f;

        if (e.type == 0)		// 通常敵
            e.x -= enemySpeed;
//        else
//..            e.x -= enemySpeed2;
		else if(e.type == 1){	  // ヘリザコ - 勢いよく突っ込む
//			static float dist_x = e.x - player_x;
            if (e.count < 24 * 2) {	// 1段階：超急接近
                 e.x -= 6 * m_deltaTime * 60.0f;
                e.y += ((playerY + 8 - e.y) / 8) / 2;// m_deltaTime; // *60.0f;// *m_deltaTime; // *60.0f;
            }
            else if (e.count < 49 * 2)	// 2段階：短くホバリング
                e.x -= 0;
            else							// 3段階：右へ全力逃走
                e.x += 6 * m_deltaTime *60.0f;
		}

		else if(e.type == 2){	  // サインカーブ
			e.x -= enemySpeed;
            e.y = (e.count2 + sin(e.count * 0.12) * 55); // *60.0f; //sin_table[e.count];
		}

//        if (e.x > SCREEN_WIDTH || e.x < 0)
//            PlaySound(m_seLaser);

        // 敵弾発射処理
        e.shootTimer += m_deltaTime;

        int difficulty = (min(1, m_gameTime / (180 * 60)));
        int enemy_bullet_speed = 4 + difficulty * 2;
        float shoot_interval = ((82 - difficulty * 36) - 5)/60;


        if (e.shootTimer >= e.nextShootTime) {


	        float dx = playerX - e.x;
	        float dy = playerY - e.y;

//            dx -= 4.0f;

	        // ベクトルの長さを計算
	        float length = sqrtf(dx * dx + dy * dy);
	        if (length > 0.001f) {   // 0除算防止
	            dx /= length;   // 正規化
	            dy /= length;


	            // 弾を発射（速度は8.0fくらいが目安）
//            difficulty = int(min(1, self.play_time / 10800))                    
//            enemy_bullet_speed = 2 + difficulty
                float bulletSpeed = enemy_bullet_speed; //4;//7.5f / 2;// / 60; //7.5f;
	            enemyBullets.push_back({
	                e.x + 16, 
	                e.y + 16,
	                dx * bulletSpeed - 1.0f*1,   // vx
	                dy * bulletSpeed    // vy
	            });
	        }

//            enemyBullets.push_back({e.x, e.y + 16});


//            PlaySound(...);

            // 次回の発射間隔を設定
//            if (e.count < 1) {                    // 最初は早めに1回
//               e.nextShootTime = 5.0f/60;           // 0.4秒間隔
 //           } else {
                e.nextShootTime = shoot_interval;//1.2f;           // その後は1.2秒間隔
 //           }

            e.shootTimer = 0.0f;
            e.count++;
        }
    }

    for (auto it = enemies.begin(); it != enemies.end(); ) {
        if ((it->x < -32) || (it->x > SCREEN_WIDTH)){// || (it->y < 32) || (it->y > SCREEN_HEIGHT)) {
            it = enemies.erase(it);
        }
        else {
            ++it;
        }
    }

    // 自機弾移動
    for (auto it = playerBullets.begin(); it != playerBullets.end(); ) {
        it->x += 13.0f * 60 * m_deltaTime;
        if ((it->x < -32) || (it->x > SCREEN_WIDTH) || (it->y < -32)|| (it->y > SCREEN_HEIGHT))  it = playerBullets.erase(it);
        else ++it;
    }
/*
    for (auto it = enemyBullets.begin(); it != enemyBullets.end(); ) {
        it->x -= 9.0f * 60 * m_deltaTime;
        if (it->x < -30) it = enemyBullets.erase(it);
        else ++it;
    }*/
    // 敵弾移動&画面範囲外判定
	for (auto it = enemyBullets.begin(); it != enemyBullets.end(); ) {
	    it->x += it->vx * m_deltaTime * 60.0f;
	    it->y += it->vy * m_deltaTime * 60.0f;

	    if ((it->x < -32) || (it->x > SCREEN_WIDTH) || (it->y < 32) || (it->y > SCREEN_HEIGHT)) {
		    it = enemyBullets.erase(it);
	    } else {
	        ++it;
	    }
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
                if (--eit->count_hp == 0) {
                    eit = enemies.erase(eit);
                    PlaySound(m_seExplosion);
                    score += 100;
                }
                else {
                    ++eit;
                }
                hit = true;
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
            m_deltaTime = static_cast<float>(now.QuadPart - last.QuadPart) / freq.QuadPart;
            last = now;

            m_accumulator += m_deltaTime;

            while (m_accumulator >= TARGET_FRAME_TIME) {
                UpdateInput();
//                if (!gameOver)
                    GameUpdate();     // deltaTimeを使って更新
//                else StarUpdate();
                m_accumulator -= TARGET_FRAME_TIME;
            }

            OnRender();           // 描画は毎回呼ぶ（垂直同期に任せる）
        }
    }
}
/*
void ShooterGame::CalculateFPS() {
    m_frameCount++;

    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);

    float elapsed = static_cast<float>(now.QuadPart - m_lastFPSTime.QuadPart) / m_freq.QuadPart;

    if (elapsed >= 0.5f) {        // 0.5秒ごとに更新（滑らかに見せる）
        float fps = m_frameCount / elapsed;

        // 移動平均を計算
        m_fpsHistory.push_back(fps);
        if (m_fpsHistory.size() > FPS_HISTORY_MAX) {
            m_fpsHistory.erase(m_fpsHistory.begin());
        }

        float sum = 0.0f;
        for (float f : m_fpsHistory) sum += f;
        m_currentFPS = sum / m_fpsHistory.size();

        m_frameCount = 0;
        m_lastFPSTime = now;
    }
}*/

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