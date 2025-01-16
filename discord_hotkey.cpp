#include <windows.h>
#include <shellapi.h>

#define WM_TRAYICON (WM_USER + 1)
#define ID_TRAYICON 1
#define ID_EXIT 2000

NOTIFYICONDATAW nid = {0};
HWND hwnd;
bool isRunning = true;
HHOOK keyboardHook;
DWORD lastKeyPress = 0;
bool isHolding = false;

// Prototipo de funciones
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);

void SimulateAltNumpad(int number) {
    keybd_event(VK_MENU, 0, 0, 0);
    keybd_event(VK_NUMPAD0 + number, 0, 0, 0);
    keybd_event(VK_NUMPAD0 + number, 0, KEYEVENTF_KEYUP, 0);
    keybd_event(VK_MENU, 0, KEYEVENTF_KEYUP, 0);
}

void CreateTrayIcon(HWND hwnd) {
    nid.cbSize = sizeof(NOTIFYICONDATAW);
    nid.hWnd = hwnd;
    nid.uID = ID_TRAYICON;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = WM_TRAYICON;
    nid.hIcon = LoadIconW(NULL, MAKEINTRESOURCEW(32512));  
    wcscpy(nid.szTip, L"Discord Mute/Deafen Tool");
    Shell_NotifyIconW(NIM_ADD, &nid);
}

LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    static bool hasTriggeredDeafen = false;

    if (nCode >= 0) {
        KBDLLHOOKSTRUCT* kbStruct = (KBDLLHOOKSTRUCT*)lParam;
        
        if (kbStruct->vkCode == VK_NUMPAD0) {
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
        case WM_TRAYICON:
            if (lParam == WM_RBUTTONUP) {
                POINT pt;
                GetCursorPos(&pt);
                HMENU menu = CreatePopupMenu();
                AppendMenuW(menu, MF_STRING, ID_EXIT, L"Exit");
                SetForegroundWindow(hwnd);
                TrackPopupMenu(menu, TPM_RIGHTALIGN | TPM_BOTTOMALIGN, pt.x, pt.y, 0, hwnd, NULL);
                DestroyMenu(menu);
            }
            break;
            
        case WM_COMMAND:
            if (LOWORD(wParam) == ID_EXIT) {
                DestroyWindow(hwnd);
            }
            break;
            
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
    WNDCLASSEXW wc = {0};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"DiscordMuteDeafenClass";
    RegisterClassExW(&wc);
    
    hwnd = CreateWindowExW(0, L"DiscordMuteDeafenClass", L"Discord Mute/Deafen Tool",
                         WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
                         400, 300, NULL, NULL, hInstance, NULL);
    
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
    
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    
    UnhookWindowsHookEx(keyboardHook);
    return (int)msg.wParam;
}
