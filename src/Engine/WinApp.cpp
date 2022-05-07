#include "WinApp.h"
#include"KuroFunc.h"

WinApp* WinApp::INSTANCE = nullptr;

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);

//システムメッセージを処理するための関数、ウィンドウプロシージャを作る
LRESULT WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    //メッセージで分岐
    switch (msg)
    {
    case WM_DESTROY: //ウィンドウが破棄された
        PostQuitMessage(0);	//OSに対してアプリの終了を伝える
        return 0;
    }
    ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam);
    return DefWindowProc(hwnd, msg, wparam, lparam);	//標準の処理を行う
}

void WinApp::Initialize(const std::string& WinName, const Vec2<int>WinSize, const bool& FullScreen, const wchar_t* IconPath = nullptr)
{
    const std::wstring winWideName = KuroFunc::GetWideStrFromStr(WinName);
    winSize = WinSize;

    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = (WNDPROC)WindowProc;
    wc.lpszClassName = winWideName.c_str();
    wc.style = CS_HREDRAW | CS_VREDRAW;
    auto hInstance = GetModuleHandle(nullptr);
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);

    //ウィンドウアイコン設定
    if (IconPath != nullptr)
    {
        wc.hIcon = (HICON)LoadImage(
            NULL, IconPath, IMAGE_ICON,
            0, 0, LR_SHARED | LR_LOADFROMFILE);
    }
    
    //ウィンドウクラスをOSに登録
    RegisterClassEx(&wc);

    RECT rect;
    DWORD dwStyle = WS_OVERLAPPEDWINDOW & ~WS_SIZEBOX;
    if (FullScreen)
    {
        // Get the settings of the primary display
        DEVMODE devMode = {};
        devMode.dmSize = sizeof(DEVMODE);
        EnumDisplaySettings(nullptr, ENUM_CURRENT_SETTINGS, &devMode);

        //ウィンドウサイズ{ X座標　Y座標　横幅　縦幅 }
        rect = 
        {
            devMode.dmPosition.x,
            devMode.dmPosition.y,
            devMode.dmPosition.x + static_cast<LONG>(devMode.dmPelsWidth),
            devMode.dmPosition.y + static_cast<LONG>(devMode.dmPelsHeight)
        };

        winSize.x = rect.right - rect.left;
        winSize.y = rect.bottom - rect.top;

        winDifferRate = winSize.Float() / WinSize.Float();
    }
    else
    {
        rect = { 0,0, winSize.x, winSize.y };
        AdjustWindowRect(&rect, dwStyle, FALSE);
    }

    hwnd = CreateWindow(wc.lpszClassName,  //クラス名
        winWideName.c_str(),    //タイトルバー
        dwStyle,    //ウィンドウスタイル
        /*CW_USEDEFAULT*/0,  //表示X座標（OSに任せる）
        /*CW_USEDEFAULT*/0,  //表示Y座標（OSに任せる）
        rect.right - rect.left, //ウィンドウ横幅
        rect.bottom - rect.top, //ウィンドウ縦幅
        nullptr,    //親ウィンドウハンドル
        nullptr,    //メニューハンドル
        hInstance,  //呼び出しアプリケーションハンドル
        nullptr);   //オプション

    //ウィンドウ表示
    if (FullScreen)
    {
        SetWindowLong(hwnd, GWL_STYLE, WS_OVERLAPPEDWINDOW & ~(WS_CAPTION | WS_MAXIMIZEBOX | WS_MINIMIZEBOX /*| WS_SYSMENU*/ | WS_THICKFRAME));

        SetWindowPos(
            hwnd,
            HWND_TOPMOST,
            rect.left,
            rect.top,
            rect.right,
            rect.bottom,
            SWP_FRAMECHANGED | SWP_NOACTIVATE);

        //ウィンドウ表示
        ShowWindow(hwnd, SW_MAXIMIZE);
    }
    else
    {
        //ウィンドウ表示
        ShowWindow(hwnd, SW_SHOW);
    }
}