// unicode support
#ifndef UNICODE
#define UNICODE
#endif

#include <windows.h>
#include <uxtheme.h>

#include <string>
#include <vector>
#include <sstream>


/*
struct for storing informations
about keyboard layout
*/
struct LayoutInfo {
    std::wstring layoutName;
    std::wstring layoutCode;
};

struct Dimensions {
    int startX;
    int startY;
    int width;
    int height;
};

HWND hwnd;


LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

std::vector<std::wstring> getAllAvailableLayouts();

std::vector<LayoutInfo> matchLayoutData(std::vector<std::wstring> &codes);

void handleHotKey(std::vector<LayoutInfo> &layouts);


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow)
{
    const wchar_t CLASS_NAME[] = L"Input Switcher";

    WNDCLASS wc = {};
    
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    
    RegisterClass(&wc);

    LOGFONT font;

    if (GetThemeSysFont(NULL, 0, &font) == FALSE) {
        MessageBox(NULL, L"Couldn't get system font", L"Error", MB_OK);
        return -1;
    } else {
        MessageBox(NULL, font.lfFaceName, L"Success", MB_OK);
    }
    
    // place window at the center of screen
    int screenWidth = GetSystemMetrics(SM_CXMAXIMIZED);
    int screenHeight = GetSystemMetrics(SM_CYMAXIMIZED);

    Dimensions windowDimensions;
    windowDimensions.width = 400;
    windowDimensions.height = 400;
    windowDimensions.startX = (screenWidth - windowDimensions.width) / 2;
    windowDimensions.startY = (screenHeight - windowDimensions.height) / 2;
    
    hwnd = CreateWindowEx(
        0,
        CLASS_NAME,
        L"Input Switcher",
        WS_OVERLAPPEDWINDOW,
        
        windowDimensions.startX, windowDimensions.startY, 
        windowDimensions.width, windowDimensions.height,

        NULL, NULL, hInstance, NULL
    );
    
    if (hwnd == NULL) {
        MessageBox(NULL, L"Couldn't create window", L"Error", MB_OK);
        return -1;
    }

    // create gui
    int margin = 20;

    RECT rect;
    if (GetClientRect(hwnd, &rect) == FALSE) {
        MessageBox(NULL, L"Couldn't get client bounds", L"Error", MB_OK);
        return -1;
    }

    int clientWidth = rect.right;
    int clientHeight = rect.bottom;

    // create button in client's centers
    // todo: replace letters in buttons with icons
    Dimensions buttonDimensions;
    buttonDimensions.width = 40;
    buttonDimensions.height = 40;
    buttonDimensions.startX = (clientWidth - buttonDimensions.width) / 2;
    buttonDimensions.startY = (clientHeight / 2) - margin - buttonDimensions.height;

    HWND hButtonRight = CreateWindowExW(
        0,
        L"BUTTON", L"R",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,

        buttonDimensions.startX, buttonDimensions.startY,
        buttonDimensions.width, buttonDimensions.height,

        hwnd, NULL, hInstance, NULL
    );

    buttonDimensions.startY = (clientHeight / 2) + margin;

    HWND hButtonLeft = CreateWindowExW(
        0,
        L"BUTTON", L"L",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,

        buttonDimensions.startX, buttonDimensions.startY,
        buttonDimensions.width, buttonDimensions.height,

        hwnd, NULL, hInstance, NULL
    );

    // ListBox for available layouts
    Dimensions availableListDimensions;
    availableListDimensions.startX = margin;
    availableListDimensions.startY = margin;
    availableListDimensions.width = ((clientWidth - buttonDimensions.width) / 2)  - (margin * 2);
    availableListDimensions.height = clientHeight - (margin * 2);

    HWND hListBoxAvailablle = CreateWindowExW(
        WS_EX_CLIENTEDGE,
        L"LISTBOX", NULL,
        WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_AUTOVSCROLL,

        availableListDimensions.startX, availableListDimensions.startY,
        availableListDimensions.width, availableListDimensions.height,

        hwnd, NULL, hInstance, NULL
    );
    
    if (hListBoxAvailablle == NULL) {
        MessageBox(hwnd, L"Couldn't create ListBox", L"Error", MB_OK);
        return -1;
    }

    // ListBox for active layouts
    Dimensions activeListDimensions;
    activeListDimensions.startX = (margin * 3) + availableListDimensions.width + buttonDimensions.width;
    activeListDimensions.startY = margin;
    activeListDimensions.width = ((clientWidth - buttonDimensions.width) / 2)  - (margin * 2);
    activeListDimensions.height = clientHeight - (margin * 2);

    HWND hListBoxActive = CreateWindowExW(
        WS_EX_CLIENTEDGE,
        L"LISTBOX", NULL,
        WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_AUTOVSCROLL,

        activeListDimensions.startX, activeListDimensions.startY,
        activeListDimensions.width, activeListDimensions.height,

        hwnd, NULL, hInstance, NULL
    );
    
    if (hListBoxActive == NULL) {
        MessageBox(hwnd, L"Couldn't create ListBox", L"Error", MB_OK);
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
    
    std::vector<std::wstring> codes = getAllAvailableLayouts();
    std::vector<LayoutInfo> layouts = matchLayoutData(codes);

    if (layouts.size() <= 0) {
        MessageBox(hwnd, L"Couldn't find any keyboard layouts", L"Warning", MB_OK);
    }

    // fill list with names of layouts
    for (int i = 0; i < layouts.size(); i++) {
        int pos = (int) SendMessage(hListBoxActive, LB_ADDSTRING, 0, (LPARAM) layouts[i].layoutName.c_str());

        //SendMessage(hListBoxActive, LB_SETITEMDATA, i, (LPARAM) i);
    }
    
    MSG msg = {};
    while(GetMessage(&msg, NULL, 0, 0) > 0) {
        if (msg.message == WM_HOTKEY) {
            handleHotKey(layouts);
        } else {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    
    // remove tray icon on program exit
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

std::vector<std::wstring> getAllAvailableLayouts()
{
    int size = GetKeyboardLayoutList(0, nullptr);
    HKL list[size];

    std::vector<std::wstring> codes;
    
    if (GetKeyboardLayoutList(size, list) <= 0) {
        MessageBox(hwnd, L"Failed to query available keyboard layouts", L"Error", MB_OK);
        return codes;
    }
    
    for (int i = 0; i < size; i++) {
        HKL current = list[i];
        
        uint64_t *data = reinterpret_cast<uint64_t*>(&current);
        
        uint32_t code = *data >> 16;

        std::stringstream stream;
        stream << std::hex << code;
        std::string result(stream.str());
        
        std::wstring str(result.begin(), result.end());

        // add leading zeroes to string
        int length = str.length();
        for (int j = 0; j < 8 - length; j++) {
            str = L"0" + str;
        }

        codes.push_back(str);
    }

    return codes;
}


/*
https://docs.microsoft.com/en-us/windows/win32/sysinfo/registry-functions
matches layout codes with their names in registry
*/
std::vector<LayoutInfo> matchLayoutData(std::vector<std::wstring> &codes)
{
    HKEY hkey;
    LSTATUS result = RegOpenKeyExW(
        HKEY_LOCAL_MACHINE,
        L"SYSTEM\\CurrentControlSet\\Control\\Keyboard Layouts",
        0, KEY_ENUMERATE_SUB_KEYS | KEY_QUERY_VALUE, &hkey
    );

    std::vector<LayoutInfo> matched;

    if (result != ERROR_SUCCESS) {
        MessageBox(hwnd, L"Error while attetmpting to open registry", L"Error", MB_OK);
        return matched;
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

        LayoutInfo info = {};
        info.layoutCode = *i;

        if (result != ERROR_SUCCESS) {
            MessageBox(hwnd, L"Failed to read value", L"Error", MB_OK);
            info.layoutName = std::wstring(L"Unknown Layout ") + info.layoutCode;
        } else {
            info.layoutName = std::wstring(data);
        }

        matched.push_back(info);
    }

    return matched;
}

void handleHotKey(std::vector<LayoutInfo> &layouts) {
    if (layouts.size() <= 0) {
        return;
    }

    HWND current = GetForegroundWindow();
    
    wchar_t code[8];
    GetKeyboardLayoutNameW(code);
    
    HKL hkl;

    // find current layout
    int index = 0;
    for (int i = 0; i != layouts.size(); i++) {
        if (layouts[i].layoutCode == code) {
            index = i + 1;
            break;
        }
    }

    if (index >= layouts.size()) {
        index = 0;
    }

    // change to next layout
    hkl = LoadKeyboardLayout(layouts[index].layoutCode.c_str(), KLF_ACTIVATE);
    
    PostMessageW(
        current,
        WM_INPUTLANGCHANGEREQUEST,
        0,
        (LPARAM) hkl
    );
}