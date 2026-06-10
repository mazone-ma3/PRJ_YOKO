// Direct2DShooter.cpp
#define WIN32_LEAN_AND_MEAN
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
#include <cmath>

#include <shellscalingapi.h>
#include <string>     // std::wstring
#include <shlwapi.h>  // Path系が必要なら

//#include <mmsystem.h>
//#pragma comment(lib, "winmm.lib")

#include <iostream>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Media.Playback.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Storage.h>
#pragma comment(lib, "windowsapp") // 必要に応じて追加
#include <winrt/Windows.Media.Core.h>
#include <thread> // ★std::threadを使うために追加

using namespace winrt;
using namespace Windows::Media::Playback;
using namespace Windows::Storage;
using namespace Windows::Media::Core;

MediaPlayer g_mediaPlayer = nullptr;

#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "shcore.lib") 
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")
#pragma comment(lib, "windowscodecs.lib")
#pragma comment(lib, "xaudio2.lib")
#pragma comment(lib, "xinput.lib")

#define SCREEN_WIDTH  (256*2)
#define SCREEN_HEIGHT (192*2)
#define COUNT1S 60.0f

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
//struct Particle { float x, y, vx, vy; int life; };

struct Option {
    float offset_y;     // 自機からの相対Y（-25 or +25 など）
    float x, y;         // 現在の位置
//    float angle;        // 回転角度（滑らかに回す用）
};

struct Item { float x, y, timer; int type; };

struct ChainItem {
    float x, y;
    float timer;        // floatに変更推奨
};

struct Star { float x, y, baseSpeed, speed, size; ID2D1SolidColorBrush* brush = nullptr; };

struct Particle {
    float x, y;
    float vx, vy;
    float life;           // 残りフレーム
    int color_index;    // 0?4で星ブラシと同じ色を使う
    int type;           // 0=通常破片、1=大きな爆発など（後で拡張用）
};

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

// 実際に再生を行う中身（別スレッドで実行される）
void AudioThreadWorker() {
    // この新しいスレッドを「MTA（マルチスレッド）」として初期化する
    winrt::init_apartment(winrt::apartment_type::multi_threaded);

    try {
        // パスは実際のファイルパスに書き換えてください
        auto file = StorageFile::GetFileFromPathAsync(m_exeDir + L"bgm.mp3").get();

        g_mediaPlayer = MediaPlayer();
        g_mediaPlayer.Source(Windows::Media::Core::MediaSource::CreateFromStorageFile(file));

        // ★ リピート再生を有効にする
        g_mediaPlayer.IsLoopingEnabled(true);

        g_mediaPlayer.Play();
    }
    catch (winrt::hresult_error const& ex) {
        // エラー処理
        OutputDebugString(ex.message().c_str());
    }
}

// ボタンを押したときや、メイン処理から呼び出す関数
void PlayMyMp3() {
    // 音楽再生用のスレッドを立ち上げて、処理を丸投げする
    std::thread t(AudioThreadWorker);

    // スレッドの管理を切り離す（これで関数が終了してもバックグラウンドで再生が続く）
    t.detach();
}

void StopMp3() {
    if (g_mediaPlayer != nullptr) {
        // 再生を一時停止
        g_mediaPlayer.Pause();

        // 再生位置を 0（曲の先頭）に戻す
        g_mediaPlayer.Position(Windows::Foundation::TimeSpan::zero());
    }
}

class SimpleBGM {
    HWND hWnd;           // 通知を受け取るウィンドウハンドル
    const wchar_t* filename =  + L"bgm.mp3";
    UINT lastNotifyTime = 0;   // 連打防止用

public:
    bool isPlaying = false;

    SimpleBGM(HWND hwnd) : hWnd(hwnd) {}

    bool Start() {
        PlayMyMp3();
//        Stop();  // 念のため閉じる

/*        char cmd[256];
        sprintf_s(cmd, "open \"%ws\" type MPEGVideo alias bgm", (m_exeDir + filename).c_str());
        if (mciSendStringA(cmd, NULL, 0, NULL) != 0) return false;

        mciSendStringA("set bgm time format milliseconds", NULL, 0, NULL);
        PlayOnce();  // 初回再生
        isPlaying = true;*/
        return true;
    }

//    void PlayOnce() {
/*//        mciSendStringA("stop bgm", NULL, 0, NULL);
        mciSendStringA("seek bgm to start", NULL, 0, NULL);
//        mciSendStringA("play bgm notify", NULL, 0, hWnd);  // notifyで終了通知
        mciSendStringA("play bgm repeat", NULL, 0, hWnd);*/
//    }

    void Stop() {
        StopMp3();
/*        mciSendStringA("stop bgm", NULL, 0, NULL);
        mciSendStringA("close bgm", NULL, 0, NULL);*/
//        isPlaying = false;
    }

    // ウィンドウプロシージャで呼ぶ
/*    void HandleNotify() {
//        if (isPlaying) {
//            PlayOnce();  // 終わったら即再再生 → ループ
//        }
        if (!isPlaying) return;

        UINT now = GetTickCount();

        // 直近300ms以内の通知は無視（連打防止）
        if (now - lastNotifyTime < 300) return;

        lastNotifyTime = now;

        // 再再生
        mciSendStringA("play bgm notify", NULL, 0, hWnd);
    }*/
};

class ShooterGame {
public:
    ShooterGame();
    ~ShooterGame();

    HRESULT Initialize();
    void RunMessageLoop();

private:
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    LRESULT HandleMessage(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

    HRESULT CreateDeviceResources();
    void DiscardDeviceResources();

    HRESULT LoadBitmapFromFile(PCWSTR uri, ID2D1Bitmap** ppBitmap);
    HRESULT LoadSound(const wchar_t* filename, SoundEffect& sound);
    void PlaySound(const SoundEffect& sound);
//    void PlayBGM(const wchar_t* filename);
    bool PlayBGM(const wchar_t* filename);
    void StopBGM();
    void CleanupVoices();
    void ToggleFullscreen();
    void UpdateInput();

	void InitStars();
    void StarUpdate();
    void GameUpdate();
    void OnRender();
    void CheckCollisions();
    void ResetGame();
    void ClampPlayer();

	bool put_sprite(float x, float y, int pat);
    void put_strings(float x, float y, wchar_t* str, int mode);
    void put_strings(float x, float y, wchar_t *str);
    void put_strings_num(float x, float y, wchar_t* str, int num, int digit, int mode);
    void put_strings_num(float x, float y, wchar_t* str, int num, int digit);

    void CreateParticles(float x, float y, int count, int type); // = 0);
	void UseBomb();

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
    ID2D1Bitmap* m_pFontBitmap = NULL;

    IXAudio2* m_pXAudio2 = NULL;
    IXAudio2MasteringVoice* m_pMasterVoice = NULL;
    IXAudio2SourceVoice* m_pBgmVoice = NULL;
    BYTE* m_pBgmBuffer = NULL;

    SoundEffect m_seLaser;
    SoundEffect m_seExplosion;
    std::vector<IXAudio2SourceVoice*> m_activeSounds;

    bool g_IsFullscreen = true;

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
    const float TARGET_FRAME_TIME = 1.0f / COUNT1S; // 60FPS目標
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
    float playerX = COUNT1S, playerY = 160.0f;
    std::vector<Bullet> playerBullets;
    std::vector<eBullets> enemyBullets;
    std::vector<Particle> Particles;
    std::vector<Option> Options;
    std::vector<Item> Items;
    std::vector<ChainItem> chain_items;   // クラス内に追加

    std::vector<Enemy> enemies;
    std::vector<Star> stars;

    std::vector<Particle> particles;

	int bomb_stock = 0;
	bool bomb_active = false;
	float bomb_timer = 0.0f;
	const float BOMB_DURATION = 0.0f;
    bool key_b_flag = false;

    bool shield_active = false;
    float shield_timer = 0.0f;     // 残り時間
    const float SHIELD_DURATION = 8.0f;   // シールド持続時間（秒）

	int chain_count = 0;
    float chain_timer = 0.0f;     // floatに変更推奨
    const float CHAIN_TIME_LIMIT = 3.5f;   // チェイン持続時間（秒）

	int option_cooldown = 10;
	int enemy_spawn_timer = 0;
	int kill_count = 0;
	int shoot_timer = 0;

	int score = 0, lives = 3, high_score=5000;
	int gameOver = 0;

//	int play_time = 0;		  // 経過時間（フレーム）


	int sx,sy,dx,dy,ex,ey,ph_x,ph_y,ph_w,ph_h;

	int enemy_bullet_speed;

	int shoot_interval;
	int dist;
	int direction_factor;
	int offset;

    const int MAX_OPTIONS = 2;   // 最大オプション数

	bool FPS_flag = false;
    bool easy_mode = false;
};

//int sin_table[256 * 4];
SimpleBGM* bgm = nullptr;

// 再生開始
bool ShooterGame::PlayBGM(const wchar_t* filename)
{
/*    char command[512];

    // まず閉じる（念のため）
    mciSendStringA("close bgm", NULL, 0, NULL);

    // ファイルを開く（MP3の場合 type MPEGVideo を明示的に指定しても良い）
    sprintf_s(command, "open \"%ws\" type MPEGVideo alias bgm", filename);
    if (mciSendStringA(command, NULL, 0, NULL) != 0) {
        // 失敗した場合
        return false;
    }

    // 時間をミリ秒単位に設定
    mciSendStringA("set bgm time format milliseconds", NULL, 0, NULL);

    // ループ再生（repeat）
    mciSendStringA("play bgm repeat", NULL, 0, NULL);*/
    bgm->Start();
    return true;
}

// 停止
void ShooterGame::StopBGM()
{
/*    mciSendStringA("stop bgm", NULL, 0, NULL);
    mciSendStringA("close bgm", NULL, 0, NULL);*/
    bgm->Stop();
}

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

//    m_exeDir = GetExeDirectory();

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
//        WS_OVERLAPPEDWINDOW,
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT,
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

//    bgm = new SimpleBGM(hwnd);

    return S_OK;
}

LRESULT CALLBACK ShooterGame::WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    ShooterGame* pThis = nullptr;
    if (message == WM_CREATE) {
        pThis = reinterpret_cast<ShooterGame*>(((LPCREATESTRUCT)lParam)->lpCreateParams);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));

//        m_exeDir = GetExeDirectory();

//        bgm = new SimpleBGM(hwnd);
//        bgm->Start();
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

    case WM_SETCURSOR:
        if (g_IsFullscreen)
        {
            // ヒットテストがクライアント領域内であればカーソルを消す
            if (LOWORD(lParam) == HTCLIENT)
            {
                SetCursor(NULL);
                return TRUE; // 処理済みとして TRUE を返す
            }
        }
        break;

    case WM_CREATE:
        bgm = new SimpleBGM(hwnd);
        break;

/*    case MM_MCINOTIFY:
//        if (bgm) bgm->HandleNotify();
        if (wParam == MCI_NOTIFY_SUCCESSFUL)
        {
            if (bgm->isPlaying) {
                // すぐに再再生（通知連打対策は最小限に）
//                mciSendStringA("play bgm notify", NULL, 0, hwnd);
                bgm->PlayOnce();
//                bgm->Start();
                return  0;
            }
            else
                return  0;
        }
        else if (wParam == MCI_NOTIFY_ABORTED) {
            // stopなどで中断された場合は、何もせず無視する
            return 0;
        }
        break;*/
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
    keys['B'] = (GetAsyncKeyState('B') & 0x8000) != 0;

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

    static bool f12PressedLast = false;
    bool f12Now = (GetAsyncKeyState(VK_F12) & 0x8000) != 0;
    if (f12Now && !f12PressedLast) {
        FPS_flag = !FPS_flag;
    }
    f12PressedLast = f12Now;

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

/*
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
*/

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

        if (!m_pFontBitmap) LoadBitmapFromFile((m_exeDir + L"FONTYOKO.png").c_str(), &m_pFontBitmap);
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
    SafeRelease(&m_pFontBitmap);

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



// ゲームリセット
void ShooterGame::ResetGame() {
    playerX = 60.f; playerY = 160.0f; //SCREEN_HEIGHT / 2 - 32;
    score = 0;
    if (easy_mode == true)
        lives = 3;
    else
        lives = 1;
    gameOver = 0;
    playerBullets.clear();
    enemyBullets.clear();
    enemies.clear();
    Options.clear();
    Items.clear();
    scrollX = 0;
    m_gameTime = 0;
    particles.clear();
    option_cooldown = 10;
	shield_active = false;
    bomb_active = false;
    bomb_stock = 0;
    key_b_flag = false;
    chain_items.clear();
    chain_count = 0;

//	InitStars();

    StopBGM();
//    PlayBGM((m_exeDir + L"bgm.wav").c_str());
    PlayBGM((m_exeDir + L"bgm.mp3").c_str());
}


void ShooterGame::StarUpdate() {
    // 星移動
    for (auto& s : stars) {
//        s.x -= s.speed * m_deltaTime * COUNT1S;   // COUNT1Sは基準FPS（調整用）
		s.x -= s.baseSpeed * m_deltaTime * COUNT1S;   // これでFPSが60の時に元の速度と同じになる
        if (s.x < 0) {
            s.x = SCREEN_WIDTH; //800;
            s.y = static_cast<float>(rand() % SCREEN_HEIGHT); //600);
        }
    }
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

void ShooterGame::ClampPlayer() {
    if (playerX < 0) playerX = 0;
    if (playerY < 0) playerY = 0;
    if (playerX > SCREEN_WIDTH-40) playerX = SCREEN_WIDTH-40;//720;
    if (playerY > SCREEN_HEIGHT-32) playerY = SCREEN_HEIGHT-32;//520;
}

// ゲーム進行
void ShooterGame::GameUpdate() {
    if (gameOver == 1)
        if(!keys[VK_SPACE] && !keys['R'] && !keys['X'] && !keys['B'])
			gameOver = 2;

	if (gameOver == 2)
        if (keys[VK_SPACE] || keys['R']) { easy_mode = false; ResetGame(); }
        else if (keys['X'] || keys['B']) { easy_mode = true; ResetGame(); }

    StarUpdate();
    if (gameOver)
		return;

    m_gameTime += m_deltaTime;

//    CleanupVoices();
	static float voiceCleanTimer = 0.0f;
	voiceCleanTimer += m_deltaTime;
	if (voiceCleanTimer > 0.5f) {     // 0.5秒ごとに掃除
	    CleanupVoices();
	    voiceCleanTimer = 0.0f;
	}

    // 敵生成頻度を時間経過で徐々に上げる
//    float progress = m_gameTime / 180.0f;           // 0.0 ~ 1.0（180秒で最大）
//    if (progress > 1.0f) progress = 1.0f;

    // 最初は敵が少なく、徐々に増える（0.8秒間隔 → 0.25秒間隔くらい）
//    m_enemySpawnRate = 0.8f - (0.55f * progress);

	float moveSpeed = 4.0f * COUNT1S * m_deltaTime;
	float enemySpeed = 4.0f * COUNT1S * m_deltaTime;
	float enemySpeed2 = 5.0f * COUNT1S * m_deltaTime;

    if (keys[VK_LEFT])  playerX -= moveSpeed;
    if (keys[VK_RIGHT]) playerX += moveSpeed;
    if (keys[VK_UP])    playerY -= moveSpeed;
    if (keys[VK_DOWN])  playerY += moveSpeed;
    ClampPlayer();

    // オプション更新
    for (auto& opt : Options) {
//        opt.angle += 0.08f * COUNT1S * m_deltaTime;   // 回転速度

        // 滑らかに追従
//        opt.x += ((playerX + 16) - opt.x) / 4 * m_deltaTime * COUNT1S;
//        opt.y += ((playerY + opt.offset_y) - opt.y) / 4 * m_deltaTime * COUNT1S;
		float t = 1.0f - pow(1.0f - 0.25f, m_deltaTime * COUNT1S);
		opt.x = std::lerp(opt.x, playerX + 16, t);
		opt.y = std::lerp(opt.y, playerY + opt.offset_y, t);
    }

// 射撃クールタイムも時間ベースに
    static float shootTimer = 0.0f;
    shootTimer += m_deltaTime;
    if (keys[VK_SPACE] && shootTimer >= 8 / COUNT1S) {
        playerBullets.push_back({ playerX + 32, playerY + 12 });

        // オプションからも発射
        for (const auto& opt : Options) {
            playerBullets.push_back({ opt.x + 8, opt.y + 12 });
        }
        shootTimer = 0.0f;
//        PlaySound(m_seLaser);
    }

	// GameUpdate()内がおすすめ
	if ((keys['X'] || keys['B']) && bomb_stock > 0 && !bomb_active) {
        if (key_b_flag == false)
            UseBomb();
        key_b_flag = true;
    }
    else{
        key_b_flag = false;
 	}

// === 敵生成処理（スコアベース）===
    static float enemySpawnTimer = 0.0f;
    enemySpawnTimer += m_deltaTime;

    // 元のロジックをdeltaTimeに変換
    float baseInterval = 50.0f - (score / 250.0f);   // scoreが増えるほど短く
    float spawnInterval = max(18 / COUNT1S, baseInterval / COUNT1S);  // フレーム→秒に変換

    if (enemySpawnTimer >= spawnInterval) {

        int type, rand_num;

		rand_num = rand() % 100;
		if(rand_num < 60) type = 0;
		else if(rand_num < 85) type = 1;
		else type = 2;

        float count = rand() % (30 * 2 + SCREEN_HEIGHT - 40 * 2) - 30 * 2;
        enemies.push_back({ SCREEN_WIDTH+0.0f, 32.0f + (rand() % (SCREEN_HEIGHT-32-32-32)), 0.0f, 5.0f / COUNT1S, type, 0.0f, count,
            (type == 0)? 1:3, false});
		enemySpawnTimer = 0.0f;
    }

    // 敵移動 + 敵弾発射
    for (auto& e : enemies) {
        e.count += m_deltaTime * COUNT1S;

        if (e.type == 0)		// 通常敵
            e.x -= enemySpeed;
		else if(e.type == 1){	  // ヘリザコ - 勢いよく突っ込む
//			static float dist_x = e.x - player_x;
            if (e.count < 24) {	// 1段階：超急接近
                e.x -= 6 * 2 * m_deltaTime * COUNT1S;
                e.y += ((playerY + 8 - e.y) / 8) / 2 * m_deltaTime * COUNT1S;
            }
            else if (e.count < 49)	// 2段階：短くホバリング
                e.x -= 0;
            else							// 3段階：右へ全力逃走
                e.x += 6 * 2 * m_deltaTime * COUNT1S;
		}

		else if(e.type == 2){	  // サインカーブ
			e.x -= enemySpeed;
            e.y = (e.count2 + sinf(e.count * 0.12) * 55 * 2);
		}

        // 敵弾発射処理
        e.shootTimer += m_deltaTime;

        int difficulty = (min(1, m_gameTime / (180))); // * COUNT1S)));
        float enemy_bullet_speed = (4 + difficulty * 2);
        float shoot_interval = ((82 - difficulty * 36) - 5) / COUNT1S;


        if (e.shootTimer >= e.nextShootTime) {

            float dx = playerX - e.x;
	        float dy = playerY - e.y;

//            dx -= 4.0f;

			float dist;
			if(abs(dx) > abs(dy))
				dist = abs(dx);
			else
				dist = abs(dy);

			if (dist == 0) dist = 1;


	        // ベクトルの長さを計算
//	        float length = sqrtf(dx * dx + dy * dy);
//	        if (length > 0.001f) {   // 0除算防止
//	            dx /= length;   // 正規化
//	            dy /= length;


	            // 弾を発射
                float bulletSpeed = enemy_bullet_speed;

				dx = (dx * bulletSpeed/dist);
				dy = (dy * bulletSpeed/dist);
				dx = max(-3*2, dx);
				dx = min(dx, 4*2);
				dy = max(-4*2, dy);
				dy = min(dy, 4*2);

	            enemyBullets.push_back({
	                e.x + 16, 
	                e.y + 16,
	                dx, // * bulletSpeed - 1.0f*1,   // vx
	                dy, // * bulletSpeed     // vy
	            });
//	        }

            // 次回の発射間隔を設定
            e.nextShootTime = shoot_interval;

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
        it->x += 12.0f *COUNT1S * m_deltaTime;
        if ((it->x < -32) || (it->x > SCREEN_WIDTH) || (it->y < -32)|| (it->y > SCREEN_HEIGHT))  it = playerBullets.erase(it);
        else ++it;
    }

    // 敵弾移動&画面範囲外判定
	for (auto it = enemyBullets.begin(); it != enemyBullets.end(); ) {
	    it->x += it->vx * m_deltaTime * COUNT1S;
	    it->y += it->vy * m_deltaTime * COUNT1S;

	    if ((it->x < -32) || (it->x > SCREEN_WIDTH) || (it->y < 32) || (it->y > SCREEN_HEIGHT)) {
		    it = enemyBullets.erase(it);
	    } else {
	        ++it;
	    }
	}

    CheckCollisions();


    // パーティクル更新
    for (auto it = particles.begin(); it != particles.end(); ) {
        it->x += it->vx * m_deltaTime * COUNT1S;
        it->y += it->vy * m_deltaTime * COUNT1S;
//        it->vx *= 0.96f;      // 少し減速（空気抵抗）
//        it->vy *= 0.96f;

			float damping = pow(0.96f, m_deltaTime * COUNT1S); 
			it->vx *= damping;
			it->vy *= damping;

        it->life -= m_deltaTime * COUNT1S;

        if (it->life <= 0) {
            it = particles.erase(it);
        } else {
            ++it;
        }
    }

    // ボム更新
    if (bomb_active) {
        bomb_timer -= m_deltaTime;
        if (bomb_timer <= 0.0f) {
            bomb_active = false;
        }
    }

    // チェインアイテム更新
    for (auto it = chain_items.begin(); it != chain_items.end(); ) {
        it->x -= 4.0f * m_deltaTime * COUNT1S;   // 左に流れる
        it->timer -= m_deltaTime;

        // 自機取得判定
        if (abs(it->x - playerX) < 44-16 && abs(it->y - playerY) < 44-16) {
            chain_count++;
            chain_timer = 240 / COUNT1S;           // チェイン持続時間リセット
            score += chain_count * 100;    // チェイン数に応じたボーナス

            it = chain_items.erase(it);
            PlaySound(m_seLaser);     // 取得音
            continue;
        }

        // 時間切れ or 画面外
        if (it->timer <= 0.0f || it->x < -20) {
            chain_count = 0;
            it = chain_items.erase(it);
        } else {
            ++it;
        }
    }

    // チェインタイマー減少
    if (chain_timer > 0.0f) {
        chain_timer -= m_deltaTime;
        if (chain_timer <= 0.0f) {
            chain_count = 0;
        }
    }
    if(gameOver && (score > high_score))
        high_score = score;

}

void ShooterGame::CheckCollisions() {
    // 自弾 vs 敵
    for (auto bit = playerBullets.begin(); bit != playerBullets.end(); ) {
        bool hit = false;
        for (auto eit = enemies.begin(); eit != enemies.end(); ) {
            if (bit->x + 16 > eit->x && bit->x < eit->x + 32 &&
                bit->y + 8> eit->y && bit->y < eit->y + 32) {

				CreateParticles(eit->x + 16, eit->y + 16, 8, 0);   // 通常爆発

                if (--eit->count_hp == 0) {

				    // オプションアイテム出現（確率20%くらい）
//				    if (rand() % 100 < 22 && Options.size() < MAX_OPTIONS) {
                    if (option_cooldown <= 0){
				        Item item;
				        item.x = eit->x;
				        item.y = eit->y;
                        item.timer = 300.0f;        // 約5秒で消える
				        item.type = 1;           // 1 = オプションアイテム
				        Items.push_back(item);
                        option_cooldown = 10;
                    }
                    else {
                        --option_cooldown;
                    }

				    // シールドアイテム出現（確率12%程度）
				    if (rand() % 100 < 12 && !shield_active) {
				        Item item;
				        item.x = eit->x;
				        item.y = eit->y;
				        item.timer = 280.0f;
				        item.type = 2;         // 2 = シールド
				        Items.push_back(item);
				    }

                    // ボムアイテム出現
                    if (rand() % 100 < 10) {        // 約10%の確率
					    Item item;
					    item.x = eit->x;
					    item.y = eit->y;
					    item.timer = 270.0f;
					    item.type = 3;              // 3 = ボム
					    Items.push_back(item);
					}
				    // === チェインアイテム出現 ===
				    if (rand() % 100 < 40) {        // 40%くらいの確率で落とす
				        ChainItem item;
				        item.x = eit->x;
				        item.y = eit->y;
				        item.timer = 240.0f;
				        chain_items.push_back(item);
				    }

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
        if (it->x > playerX && it->x + 8 < playerX + 32 &&
            it->y > playerY + 6 && it->y + 8 < playerY + 6 + 20) {

	        if (shield_active) {
	            shield_active = false;           // シールド消費
	            CreateParticles(playerX + 16, playerY + 16, 18, 1); // 大きな爆発
	        } else {
	            lives--;
	            if (lives <= 0) {
	                gameOver = 1;
	                StopBGM();
	            }
			}
            it = enemyBullets.erase(it);
            break;
        }
        else ++it;
    }

    // 敵 vs 自機
    for (auto it = enemies.begin(); it != enemies.end(); ) {
        if (it->x + 32 > playerX && it->x < playerX + 32 &&
            it->y + 32 > playerY + 6 && it->y < playerY + 6 + 20) {
            if (shield_active) {
                shield_active = false;           // シールド消費
                CreateParticles(playerX + 16, playerY + 16, 18, 1); // 大きな爆発
            }
            else {
                lives--;
                if (lives <= 0) {
                    gameOver = 1;
                    StopBGM();
                    return;
                }
            }
            it = enemies.erase(it);
            break;
        }
        else ++it;
    }
 
    // アイテム更新
    for (auto it = Items.begin(); it != Items.end(); ) {
        if (it->type == 1) {
            it->x -= 2.0f * COUNT1S * m_deltaTime;   // 左に流れる
        }
        else if (it->type == 2) {
            it->x -= 4.0f * COUNT1S * m_deltaTime;   // 左に流れる
        }
        else if (it->type == 3) {
            it->x -= 4.0f * COUNT1S * m_deltaTime;   // 左に流れる
        }
        it->timer -= m_deltaTime;

        // 自機との当たり判定
        if (abs(it->x - playerX) < 44-16 && abs(it->y - playerY) < 44-16) {

            if (it->type == 1 && Options.size() < MAX_OPTIONS) {   // オプションアイテム
                float offset = (Options.size() == 0) ? 25.0f : -25.0f;
                Option opt;
                opt.offset_y = offset*2;
                opt.x = 0;//playerX + 20;
                opt.y = 0;//playerY + 16 + offset;
//                opt.angle = 0.0f;
                Options.push_back(opt);
            }
            else if (it->type == 2) {                    // シールド
                shield_active = true;
                shield_timer = SHIELD_DURATION;
            }
			else if (it->type == 3) {        // 3 = ボムアイテム
			    bomb_stock = min(3, bomb_stock + 1);
			}

            PlaySound(m_seLaser);
            it = Items.erase(it);
            continue;
        }

        // 画面外 or 時間切れ
        if (it->x < -40 || it->timer <= 0) {
            it = Items.erase(it);
        } else {
            ++it;
        }
    }

    // シールドタイマー更新
/*    if (shield_active) {
        shield_timer -= m_deltaTime;
        if (shield_timer <= 0.0f) {
            shield_active = false;
        }
    }*/
}



void ShooterGame::UseBomb() {
    if (bomb_stock <= 0 || bomb_active) return;

    bomb_stock--;
    bomb_active = true;
    bomb_timer = BOMB_DURATION;

    // 敵と敵弾を全滅
    enemies.clear();
    enemyBullets.clear();

    // 大量の破片を発生
    CreateParticles(playerX + 16, playerY + 16, 45, 1);   // 大爆発

    // 画面全体に破片を散らす
    for (int i = 0; i < 60; ++i) {
        float rx = rand() % SCREEN_WIDTH;
        float ry = rand() % SCREEN_HEIGHT;
        CreateParticles(rx, ry, 6, 1);
    }

    score += 200;
//    PlaySound(m_seExplosion);   // ボム音（大きめの効果音を使う）
}


bool ShooterGame::put_sprite(float x, float y, int pat_no) {
    D2D1_RECT_F destrect = D2D1::RectF(x, y, x + 31, y + 31);
    D2D1_RECT_F sourceRect = D2D1::RectF(32 * pat_no, 0, 32 * pat_no + 31, 31);
    if (m_pSpriteBitmap) {
        m_pRenderTarget->DrawBitmap(m_pSpriteBitmap, destrect, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR, sourceRect);
		return true;
    }
	return false;
}

void ShooterGame::put_strings(float x, float y, wchar_t *text, int mode) {
	int len=wcslen(text);

    if (m_pFontBitmap && !mode) {
        D2D1_RECT_F destrect, sourceRect;
		for(int i = 0; i < len; ++i){
            if (text[i] != ' ') {
                int pat_no = text[i] - '0';
                destrect = D2D1::RectF(x, y, x + 15, y + 15);
                sourceRect = D2D1::RectF(16 * (pat_no % 16), 16 * (pat_no / 16), 16 * (pat_no % 16) + 15, 16 * (pat_no / 16) + 15);
                m_pRenderTarget->DrawBitmap(m_pFontBitmap, destrect, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR, sourceRect);
            }
            x += 16;
		}
//		return true;
    }else{
        m_pRenderTarget->DrawText(text, len, m_pTextFormat, 
			D2D1::RectF(x, y, x+16*len, y+16), m_pTextBrush);
//		return false;
	}
}

void ShooterGame::put_strings(float x, float y, wchar_t* text) {
    put_strings(x, y, text, 0);
}

void ShooterGame::put_strings_num(float x, float y, wchar_t *str, int num, int digit, int mode) {
    wchar_t text[128];
    int len = wcslen(str), i = digit, j = num;
//    swprintf_s(text, L"%s%d", str, num);
	put_strings(x, y, str);

    while (i--) {
        text[i] = j % 10 + '0';
        j /= 10;
    }
    text[digit] = '\0';
    put_strings(x+len*16, y, text);
}

void ShooterGame::put_strings_num(float x, float y, wchar_t* str, int num, int digit) {
    put_strings_num(x, y, str, num, digit, 0);
}

void ShooterGame::CreateParticles(float x, float y, int count, int type = 0) {
    for (int i = 0; i < count; ++i) {
        Particle p;
        p.x = x;
        p.y = y;
        p.vx = (rand() % 100 - 50) * 0.12f;   // -6.0 ~ +6.0
        p.vy = (rand() % 100 - 50) * 0.12f;
        p.life = 20.0f + (rand() % 25);
        p.color_index = rand() % 5;
        p.type = type;
        particles.push_back(p);
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

	// パーティクル描画（星の後くらいがおすすめ）
	if (!particles.empty() && m_pTextBrush) {
	    for (const auto& p : particles) {
	        if (p.life > 0) {
	            D2D1_RECT_F rect = D2D1::RectF(p.x, p.y, p.x + 3, p.y + 3);
	            
	            if (p.color_index < 5 && m_starBrushes[p.color_index]) {
	                m_pRenderTarget->FillRectangle(rect, m_starBrushes[p.color_index]);
	            } else {
	                m_pRenderTarget->FillRectangle(rect, m_pTextBrush);
	            }
	        }
	    }
	}

    // チェインアイテム描画
    for (const auto& item : chain_items) {
        put_sprite(item.x, item.y, 3);   // 3番パターンにチェインアイテムの画像を入れる
    }

    for (const auto& i : Items) {
        if (i.type == 1)
            put_sprite(i.x, i.y, 8);
        else if (i.type == 2)
            put_sprite(i.x, i.y, 7);
        else if (i.type == 3)
            put_sprite(i.x, i.y, 9);
    }

    // オプション描画
    for (const auto& opt : Options) {
        put_sprite(opt.x, opt.y, 10);   // 10 = オプションのパターン番号（要調整）
    }

    // 敵弾
    for (const auto& b : enemyBullets) {
        put_sprite(b.x, b.y, 0);
        //            m_pRenderTarget->FillRectangle(D2D1::RectF(b.x, b.y - 3, b.x + 6, b.y + 3), m_pBulletBrush);
        /*            D2D1_RECT_F rect = D2D1::RectF(b.x, b.y, b.x + 31, b.y + 31);
                    D2D1_RECT_F sourceRect = D2D1::RectF(32 * 0, 0, 32 * 0 + 31, 31);
                    m_pRenderTarget->DrawBitmap(m_pSpriteBitmap, rect, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR, sourceRect);*/
    }
    //    }

    // 敵
//    if (m_pEnemyBrush || m_pSpriteBitmap) {
        for (const auto& e : enemies) {
			if(put_sprite(e.x, e.y, 2) == false){
            D2D1_RECT_F rect = D2D1::RectF(e.x, e.y, e.x + 31, e.y + 31);
/*            D2D1_RECT_F sourceRect = D2D1::RectF(32 * 2, 0, 32 * 2 + 31, 31);
            if (m_pSpriteBitmap) {
                m_pRenderTarget->DrawBitmap(m_pSpriteBitmap, rect, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR, sourceRect);
            }*/
                if (m_pEnemyBrush) {
                    m_pRenderTarget->FillRectangle(rect, m_pEnemyBrush);
                }
            }
        }
//    }

    // 自機弾
//    if (m_pBulletBrush) {
//    if (m_pSpriteBitmap) {
        for (const auto& b : playerBullets) {
			put_sprite(b.x, b.y, 4);
            //        m_pRenderTarget->FillRectangle(D2D1::RectF(b.x, b.y - 3, b.x + 8, b.y + 3), m_pBulletBrush);
/*            D2D1_RECT_F rect = D2D1::RectF(b.x, b.y, b.x + 31, b.y + 31);
            D2D1_RECT_F sourceRect = D2D1::RectF(32 * 4, 0, 32 * 4 + 31, 31);
            m_pRenderTarget->DrawBitmap(m_pSpriteBitmap, rect, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR, sourceRect);*/
        }

    // シールド描画
        if (shield_active && m_pSpriteBitmap) {
            // 自機の周りにバリアを表示
            D2D1_RECT_F rect = D2D1::RectF(playerX - 8, playerY - 8, playerX + 40, playerY + 40);
            D2D1_RECT_F sourceRect = D2D1::RectF(32 * 6, 0, 32 * 7, 32);   // 6番パターンにシールド画像を入れる
            m_pRenderTarget->DrawBitmap(m_pSpriteBitmap, rect, 0.7f, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR, sourceRect);

            //		put_sprite(playerX, playerY, 6);
        }

        // 自機
    //    if (m_pSpriteBitmap) {
        put_sprite(playerX, playerY, 1);
        /*        D2D1_RECT_F rect = D2D1::RectF(playerX, playerY, playerX + 31, playerY + 31);
                D2D1_RECT_F sourceRect = D2D1::RectF(32 * 1, 0, 32 * 1 + 31, 31);
                m_pRenderTarget->DrawBitmap(m_pSpriteBitmap, rect, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR, sourceRect);*/
                //    }


    // UI
    // FPS表示（右上に2段で表示）
    if (m_pTextFormat && m_pTextBrush && (FPS_flag == true)) {
        wchar_t text[64];

        swprintf_s(text, L"UPDATE %.1f", m_updateFPS);
		put_strings(SCREEN_WIDTH - 180, SCREEN_HEIGHT - 32 - 16, text, 1);

//        m_pRenderTarget->DrawText(text, wcslen(text), m_pTextFormat,
//            D2D1::RectF(SCREEN_WIDTH - 180, 8, SCREEN_WIDTH, 40), m_pTextBrush);

        swprintf_s(text, L"RENDER %.1f", m_renderFPS);
		put_strings(SCREEN_WIDTH - 180, SCREEN_HEIGHT - 16 - 16, text, 1);
//        m_pRenderTarget->DrawText(text, wcslen(text), m_pTextFormat,
//            D2D1::RectF(SCREEN_WIDTH - 180, 35, SCREEN_WIDTH, 70), m_pTextBrush);
    }
    if (m_pTextFormat && m_pTextBrush) {
        if (score >= high_score)
            put_strings_num(0, 0, const_cast<wchar_t*>(L"HIGH  "), score, 7);
        else
            put_strings_num(0, 0, const_cast<wchar_t *>(L"SCORE "), score, 7);
        if(easy_mode == true)
    		put_strings_num(0, 2*16, const_cast<wchar_t *>(L"LIVES "), lives, 1);

        put_strings_num(0, 1*16, const_cast<wchar_t *>(L"BOMB  "), bomb_stock , 1);
//        put_strings_num(16*16, 0, const_cast<wchar_t*>L"COUNT: ");


        wchar_t text[128];
/*	    swprintf_s(text, L"SCORE: %d", score);
	    m_pRenderTarget->DrawText(text, wcslen(text), m_pTextFormat, 
			D2D1::RectF(0, 0, 160, 16), m_pTextBrush);

	    swprintf_s(text, L"LIVES: %d", lives);
	    m_pRenderTarget->DrawText(text, wcslen(text), m_pTextFormat,
	        D2D1::RectF(0, 16, 160, 32), m_pTextBrush);
*/
        // TIME（LIVESの下）
//      swprintf_s(text, L"COUNT %.0f", m_gameTime);
//		put_strings(16*16, 0, text);
        put_strings_num(16 * 16, 0, const_cast<wchar_t *>(L"COUNT "), m_gameTime, 7);

/*        m_pRenderTarget->DrawText(text, wcslen(text), m_pTextFormat,
            D2D1::RectF(10, 80, 300, 110), m_pTextBrush);*/


        if (chain_count > 0) {
            put_strings_num(16*16, 1*16, const_cast<wchar_t *>(L"CHAIN "), chain_count, 3);
        }
    }

    if (gameOver && m_pTextFormat && m_pEnemyBrush) {
		put_strings(11*16, 12*16, const_cast<wchar_t *>(L"GAME OVER"));
//        m_pRenderTarget->DrawText(L"GAME OVER", 9, m_pTextFormat, 
//			D2D1::RectF(11*16, 12*16, 11*16+9*16, 12*16+16), m_pEnemyBrush);
        put_strings_num(7*16, 15*16, const_cast<wchar_t *>(L"HIGH SCORE "), high_score, 7);

        if (m_pTextBrush) {
			put_strings(7*16, 18*16, const_cast<wchar_t *>(L"PRESS A TO RESTART"));
/*	        m_pRenderTarget->DrawText(L"PRESS A TO RESTART", 18, m_pTextFormat,
                D2D1::RectF(7*16, 18*16, 7* 16 +16*18, 18*16+16), m_pTextBrush);
*/
        }
    }


    HRESULT hr = m_pRenderTarget->EndDraw();
    if (hr == D2DERR_RECREATE_TARGET){ //} || hr == D2DERR_DEVICE_LOST) {
        DiscardDeviceResources();
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

//            m_accumulator += m_deltaTime;

//            while (m_accumulator >= TARGET_FRAME_TIME) {
                UpdateInput();
                GameUpdate();     // deltaTimeを使って更新
//                m_accumulator -= TARGET_FRAME_TIME;
//            }

            OnRender();           // 描画は毎回呼ぶ（垂直同期に任せる）
        }
    }
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    // DPI Awareを最優先で設定
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

    CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

    m_exeDir = GetExeDirectory();


    ShooterGame game;
    if (SUCCEEDED(game.Initialize())) {
        game.RunMessageLoop();
    }
    CoUninitialize();
    return 0;
}