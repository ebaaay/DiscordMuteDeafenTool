#include <windows.h>
#include <shellapi.h>
#include <commctrl.h>
#pragma comment(lib, "comctl32.lib")

#define WM_TRAYICON (WM_USER + 1)
#define ID_TRAYICON 1
#define ID_EXIT 2000
#define ID_SHOW 2001
#define ID_HOTKEY 100
#define ID_APPLY 101

NOTIFYICONDATAW nid = {0};
HWND hwnd, hHotkey, hApply;
bool isRunning = true;
HHOOK keyboardHook;
DWORD lastKeyPress = 0;
bool isHolding = false;
int currentHotkey = VK_NUMPAD0;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);

void SimulateAltNumpad(int number) {
    keybd_event(VK_MENU, 0, 0, 0);
    keybd_event(VK_NUMPAD0 + number, 0, 0, 0);
    keybd_event(VK_NUMPAD0 + number, 0, KEYEVENTF_KEYUP, 0);
    keybd_event(VK_MENU, 0, KEYEVENTF_KEYUP, 0);
}

void ShowWindow() {
    ShowWindow(hwnd, SW_SHOW);
    SetForegroundWindow(hwnd);
}

void CreateTrayIcon(HWND hwnd) {
    nid.cbSize = sizeof(NOTIFYICONDATAW);
    nid.hWnd = hwnd;
    nid.uID = ID_TRAYICON;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = WM_TRAYICON;
    nid.hIcon = LoadIconW(NULL, MAKEINTRESOURCEW(32512));
    wcscpy(nid.szTip, L"Discord Mute/Deafen Tool - Doble click para abrir");
    Shell_NotifyIconW(NIM_ADD, &nid);
}

void CreateControls(HWND hwnd) {
    CreateWindowW(L"STATIC", L"Selecciona la tecla de atajo:",
                 WS_VISIBLE | WS_CHILD,
                 10, 10, 200, 20,
                 hwnd, NULL, NULL, NULL);

    hHotkey = CreateWindowW(HOTKEY_CLASS, L"",
                          WS_VISIBLE | WS_CHILD | WS_BORDER,
                          10, 40, 200, 25,
                          hwnd, (HMENU)ID_HOTKEY, NULL, NULL);

    SendMessage(hHotkey, HKM_SETHOTKEY, MAKEWORD(currentHotkey, 0), 0);

    hApply = CreateWindowW(L"BUTTON", L"Aplicar",
                         WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                         10, 80, 100, 30,
                         hwnd, (HMENU)ID_APPLY, NULL, NULL);
}

void UpdateHotkey() {
    WORD hotkeyValue = LOWORD(SendMessage(hHotkey, HKM_GETHOTKEY, 0, 0));
    currentHotkey = LOBYTE(hotkeyValue);
}

bool IsWindowVisible() {
    return IsWindowVisible(hwnd);
}

LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    static bool hasTriggeredDeafen = false;

    if (nCode >= 0) {
        KBDLLHOOKSTRUCT* kbStruct = (KBDLLHOOKSTRUCT*)lParam;
        
        if (kbStruct->vkCode == currentHotkey) {
            if (wParam == WM_KEYDOWN && !isHolding) {
                lastKeyPress = GetTickCount();
                isHolding = true;
                hasTriggeredDeafen = false;
            }
            else if (wParam == WM_KEYUP) {
                DWORD currentTime = GetTickCount();
                if (currentTime - lastKeyPress < 100) {
                    SimulateAltNumpad(9);
                }
                isHolding = false;
                hasTriggeredDeafen = false;
            }
            
            if (isHolding && !hasTriggeredDeafen && GetTickCount() - lastKeyPress >= 100) {
                SimulateAltNumpad(8);
                hasTriggeredDeafen = true;
            }
            
            return 1;
        }
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE:
            CreateControls(hwnd);
            break;

        case WM_TRAYICON:
            if (lParam == WM_LBUTTONDBLCLK) {
                if (!IsWindowVisible()) {
                    ShowWindow();
                }
            }
            else if (lParam == WM_RBUTTONUP) {
                POINT pt;
                GetCursorPos(&pt);
                HMENU menu = CreatePopupMenu();
                
                if (!IsWindowVisible()) {
                    AppendMenuW(menu, MF_STRING, ID_SHOW, L"Mostrar");
                    AppendMenuW(menu, MF_SEPARATOR, 0, NULL);
                }
                
                AppendMenuW(menu, MF_STRING, ID_EXIT, L"Salir");
                SetForegroundWindow(hwnd);
                TrackPopupMenu(menu, TPM_RIGHTALIGN | TPM_BOTTOMALIGN, pt.x, pt.y, 0, hwnd, NULL);
                DestroyMenu(menu);
            }
            break;
            
        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case ID_EXIT:
                    DestroyWindow(hwnd);
                    break;
                case ID_SHOW:
                    if (!IsWindowVisible()) {
                        ShowWindow();
                    }
                    break;
                case ID_APPLY:
                    UpdateHotkey();
                    MessageBoxW(hwnd, L"Tecla actualizada correctamente", L"Éxito", MB_OK | MB_ICONINFORMATION);
                    break;
            }
            break;
            
        case WM_CLOSE:
            ShowWindow(hwnd, SW_HIDE);
            return 0;
            
        case WM_DESTROY:
            Shell_NotifyIconW(NIM_DELETE, &nid);
            PostQuitMessage(0);
            break;
            
        default:
            return DefWindowProcW(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow) {
    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_HOTKEY_CLASS;
    InitCommonControlsEx(&icex);
    
    WNDCLASSEXW wc = {0};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = L"DiscordMuteDeafenClass";
    RegisterClassExW(&wc);
    
    hwnd = CreateWindowExW(0, L"DiscordMuteDeafenClass", L"Discord Mute/Deafen Tool",
                         WS_OVERLAPPEDWINDOW & ~(WS_MAXIMIZEBOX | WS_THICKFRAME),
                         CW_USEDEFAULT, CW_USEDEFAULT,
                         250, 160, NULL, NULL, hInstance, NULL);
    
    if (!hwnd) {
        return 1;
    }
    
    CreateTrayIcon(hwnd);
    
    keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, hInstance, 0);
    
    if (!keyboardHook) {
        return 1;
    }

    HKEY hKey;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_SET_VALUE, &hKey) == ERROR_SUCCESS) {
        WCHAR path[MAX_PATH];
        if (GetModuleFileNameW(NULL, path, MAX_PATH)) {
            RegSetValueExW(hKey, L"DiscordMuteDeafen", 0, REG_SZ, (BYTE*)path, (wcslen(path) + 1) * sizeof(WCHAR));
        }
        RegCloseKey(hKey);
    }
    
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);
    
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    
    UnhookWindowsHookEx(keyboardHook);
    return (int)msg.wParam;
}
