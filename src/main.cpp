// unicode support
#ifndef UNICODE
#define UNICODE
#endif

#include <windows.h>
#include <string>
#include <vector>
#include <sstream>

/*
    struct for storing informations
    about keyboard layout
*/
struct LayoutInfo {
    WCHAR layoutName;
    WCHAR layoutCode;
};

HWND hwnd;

// https://docs.microsoft.com/en-us/windows/win32/sysinfo/registry-functions
void matchLayoutData(std::vector<std::wstring> codes)
{
    HKEY hkey;
    LSTATUS result = RegOpenKeyExW(
        HKEY_LOCAL_MACHINE,
        L"SYSTEM\\CurrentControlSet\\Control\\Keyboard Layouts",
        0,
        KEY_ENUMERATE_SUB_KEYS | KEY_QUERY_VALUE,
        &hkey
    );

    if (result != ERROR_SUCCESS) {
        MessageBox(hwnd, L"Error while attetmpting to open registry", L"Error", MB_OK);
        return;
    }

    for (std::vector<std::wstring>::const_iterator i = codes.begin(); i < codes.end(); i++) {
        DWORD length = 255;

        wchar_t data[length];

        result = RegGetValueW(
            hkey,
            i->c_str(),
            L"Layout Text",
            RRF_RT_REG_SZ,
            NULL,
            data,
            &length
        );

        if (result != ERROR_SUCCESS) {
            MessageBox(hwnd, L"Failed to read value", L"Error", MB_OK);
            return;
        }

        std::wstring str(*i);
        str += L": ";
        str += data;

        MessageBox(hwnd, str.c_str(), L"regisry", MB_OK);

        // todo: make a vector of layout info and return
    }
}

void getAllAvailableLayouts()
{
    int size = GetKeyboardLayoutList(0, nullptr);
    HKL list[size];
    
    if (GetKeyboardLayoutList(size, list) <= 0) {
        MessageBox(hwnd, L"Failed to query available keyboard layouts", L"Error", MB_OK);
    }

    std::vector<std::wstring> codes;
    
    for (int i = 0; i < size; i++) {
        HKL current = list[i];
        
        uint64_t *data = reinterpret_cast<uint64_t*>(&current);
        
        uint32_t code = *data >> 16;

        std::stringstream stream;
        stream << std::hex << code;
        std::string result(stream.str());
        
        std::wstring str(result.begin(), result.end());

        int length = str.length();

        for (int j = 0; j < 8 - length; j++) {
            str = L"0" + str;
        }

        codes.push_back(str);
    }

    matchLayoutData(codes);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow)
{
    const wchar_t CLASS_NAME[] = L"Input Switcher";

    WNDCLASS wc = {};
    
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    
    RegisterClass(&wc);
    
    // place window at the center of screen
    int screenWidth = GetSystemMetrics(SM_CXMAXIMIZED);
    int screenHeight = GetSystemMetrics(SM_CYMAXIMIZED);
    
    int windowWidth = 200;
    int windowHeight = 200;
    
    int startX = (screenWidth - windowWidth) / 2;
    int startY = (screenHeight - windowHeight) / 2;
    
    hwnd = CreateWindowEx(
        0,
        CLASS_NAME,
        L"Input Switcher",
        WS_OVERLAPPEDWINDOW,
        
        startX, startY, windowWidth, windowHeight,
            
        NULL,
        NULL,
        hInstance,
        NULL
    );
    
    if (hwnd == NULL) {
        MessageBox(NULL, L"Couldn't create window", L"Error", MB_OK);
        return -1;
    }
    
    // register hot key for CapsLock
    // todo: make the key configurable
    if (!RegisterHotKey(hwnd, 0, 0, VK_CAPITAL)) {
        MessageBox(NULL, L"Couldn't regiser hot key", L"Error", MB_OK);
        return -1;
    }
    
    ShowWindow(hwnd, nCmdShow);
    
    // setup tray icon
    // uses default Windows information icon
    // todo: make a proper icon
    NOTIFYICONDATAW nid = {};
    nid.cbSize = sizeof(NOTIFYICONDATAW);
    nid.hWnd = hwnd;
    nid.uID = 0x10;
    nid.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE;
    nid.hIcon = LoadIcon(NULL, IDI_INFORMATION);
    
    Shell_NotifyIconW(NIM_ADD, &nid);
    
    std::vector<LayoutInfo> activeLayouts;
    
    getAllAvailableLayouts();
    
    MSG msg = {};
    while(GetMessage(&msg, NULL, 0, 0) > 0) {
        if (msg.message == WM_HOTKEY) {
            HWND current = GetForegroundWindow();
            
            wchar_t name[8];
            GetKeyboardLayoutNameW(name);
            
            // ru: 00000419
            // en: 00000409
            // jp: 00000411
            
            std::wstring nameStr(name);
            
            HKL hkl;
            if (nameStr.compare(std::wstring(L"00000411")) == 0) {
                hkl = LoadKeyboardLayout(L"00000419", KLF_ACTIVATE);
            } else if (nameStr.compare(std::wstring(L"00000419")) == 0) {
                hkl = LoadKeyboardLayout(L"00000411", KLF_ACTIVATE);
            } else {
                hkl = LoadKeyboardLayout(L"00000411", KLF_ACTIVATE);
            }
            
            PostMessageW(
                current,
                WM_INPUTLANGCHANGEREQUEST,
                0,
                (LPARAM) hkl
            );
        } else {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    
    Shell_NotifyIconW(NIM_DELETE, &nid);
    return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch(uMsg) {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    case WM_PAINT:
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        FillRect(hdc, &ps.rcPaint, (HBRUSH) (COLOR_WINDOW));
        
        EndPaint(hwnd, &ps);
        
        return 0;
    }
    
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}