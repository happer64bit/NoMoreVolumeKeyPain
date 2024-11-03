#include <windows.h>
#include <iostream>
#include <mmdeviceapi.h>
#include <endpointvolume.h>
#include <string>

#define WM_TRAYICON (WM_USER + 1)
#define ID_TRAY_EXIT 1001

// Global variables
HHOOK hKeyboardHook;
HWND hOverlayWnd = nullptr;
NOTIFYICONDATA nid = {};

// Forward declarations
void ShowOverlay(int volume);
void ChangeVolume(float change);
void CreateTrayIcon(HWND hwnd);
void RemoveTrayIcon();
void ShowContextMenu(HWND hwnd);

// Overlay window procedure
LRESULT CALLBACK OverlayWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (uMsg == WM_TIMER) {
        ShowWindow(hwnd, SW_HIDE);
        KillTimer(hwnd, 1);
        return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

// Create and display the overlay window
void ShowOverlay(int volume) {
    if (!hOverlayWnd) {
        const wchar_t CLASS_NAME[] = L"NoMoreVolumeKeyPainOverlay";
        WNDCLASS wc = {};
        wc.lpfnWndProc = OverlayWndProc;
        wc.hInstance = GetModuleHandle(nullptr);
        wc.lpszClassName = CLASS_NAME;

        RegisterClass(&wc);
        hOverlayWnd = CreateWindowEx(
            WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
            CLASS_NAME,
            L"No More Volume Key Pain",
            WS_POPUP, 100, 100, 200, 100,
            nullptr, nullptr, wc.hInstance, nullptr
        );
        SetLayeredWindowAttributes(hOverlayWnd, RGB(0, 0, 0), 100, LWA_COLORKEY);
    }

    std::wstring message = L"Volume: " + std::to_wstring(volume) + L"%";
    HDC hdc = GetDC(hOverlayWnd);
    RECT rect;
    GetClientRect(hOverlayWnd, &rect);
    FillRect(hdc, &rect, (HBRUSH)(0x000000 + 1));
    SetTextColor(hdc, RGB(255, 255, 255));
    SetBkMode(hdc, TRANSPARENT);
    DrawTextW(hdc, message.c_str(), -1, &rect, DT_CENTER | DT_SINGLELINE | DT_VCENTER);
    ReleaseDC(hOverlayWnd, hdc);
    ShowWindow(hOverlayWnd, SW_SHOW);
    SetWindowPos(hOverlayWnd, nullptr, 100, 100, 300, 100, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
    SetTimer(hOverlayWnd, 1, 1000, nullptr);
}

// Function to change the system volume
void ChangeVolume(float change) {
    CoInitialize(nullptr);
    IMMDeviceEnumerator* deviceEnumerator = nullptr;
    IMMDevice* defaultDevice = nullptr;
    IAudioEndpointVolume* endpointVolume = nullptr;

    if (CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_INPROC_SERVER, __uuidof(IMMDeviceEnumerator), (void**)&deviceEnumerator) == S_OK) {
        if (deviceEnumerator->GetDefaultAudioEndpoint(eRender, eMultimedia, &defaultDevice) == S_OK) {
            if (defaultDevice->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_INPROC_SERVER, nullptr, (void**)&endpointVolume) == S_OK) {
                float currentVolume = 0.0f;
                endpointVolume->GetMasterVolumeLevelScalar(&currentVolume);
                float newVolume = currentVolume + change;
                if (newVolume < 0.0f) newVolume = 0.0f;
                if (newVolume > 1.0f) newVolume = 1.0f;
                endpointVolume->SetMasterVolumeLevelScalar(newVolume, nullptr);
                ShowOverlay(static_cast<int>(newVolume * 100));
            }
            endpointVolume->Release();
            defaultDevice->Release();
        }
        deviceEnumerator->Release();
    }
    CoUninitialize();
}

// Keyboard hook procedure
LRESULT CALLBACK KeyboardHookProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION) {
        KBDLLHOOKSTRUCT* pKeyBoard = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);
        if (wParam == WM_KEYDOWN) {
            if (pKeyBoard->vkCode == VK_F6) {
                ChangeVolume(-0.1f);
                return 1;
            } else if (pKeyBoard->vkCode == VK_F7) {
                ChangeVolume(0.1f);
                return 1;
            }
        }
    }
    return CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);
}

// System tray icon setup
void CreateTrayIcon(HWND hwnd) {
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = hwnd;
    nid.uID = 1;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = WM_TRAYICON;
    nid.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    wcscpy_s(nid.szTip, L"No More Volume Key Pain");
    Shell_NotifyIcon(NIM_ADD, &nid);
}

void RemoveTrayIcon() {
    Shell_NotifyIcon(NIM_DELETE, &nid);
}

void ShowContextMenu(HWND hwnd) {
    POINT pt;
    GetCursorPos(&pt);
    HMENU hMenu = CreatePopupMenu();
    if (hMenu) {
        InsertMenu(hMenu, -1, MF_BYPOSITION, ID_TRAY_EXIT, L"Quit");
        SetForegroundWindow(hwnd);
        TrackPopupMenu(hMenu, TPM_BOTTOMALIGN | TPM_LEFTALIGN, pt.x, pt.y, 0, hwnd, nullptr);
        DestroyMenu(hMenu);
    }
}

// Window procedure for tray icon events
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (uMsg == WM_TRAYICON && LOWORD(lParam) == WM_RBUTTONUP) {
        ShowContextMenu(hwnd);
    } else if (uMsg == WM_COMMAND && LOWORD(wParam) == ID_TRAY_EXIT) {
        PostQuitMessage(0);
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"NoMoreVolumeKeyPainTray";

    RegisterClass(&wc);
    HWND hwnd = CreateWindowEx(0, wc.lpszClassName, L"No More Volume Key Pain", 0, 0, 0, 0, 0, HWND_MESSAGE, nullptr, wc.hInstance, nullptr);
    if (hwnd == nullptr) {
        return 1;
    }

    CreateTrayIcon(hwnd);

    hKeyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardHookProc, nullptr, 0);
    if (hKeyboardHook == nullptr) {
        RemoveTrayIcon();
        return 1;
    }

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    UnhookWindowsHookEx(hKeyboardHook);
    RemoveTrayIcon();
    return 0;
}
