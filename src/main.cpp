// unicode support
#ifndef UNICODE
#define UNICODE
#endif

#include <windows.h>
#include <uxtheme.h>

#include <string>
#include <vector>
#include <sstream>


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

HWND hWindow;

HWND hButtonRight;
HWND hButtonLeft;
HWND hListBoxAvailable;
HWND hListBoxActive;

std::vector<LayoutInfo> layouts;
std::vector<LayoutInfo> activeLayouts;


LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

std::vector<std::wstring> getAllAvailableLayouts();

std::vector<LayoutInfo> matchLayoutData(std::vector<std::wstring> &codes);

void onHotKey();

bool onCommand(HWND windowHandle, int command);

void refreshActiveList();

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
    }
    
    // place window at the center of screen
    int screenWidth = GetSystemMetrics(SM_CXMAXIMIZED);
    int screenHeight = GetSystemMetrics(SM_CYMAXIMIZED);

    Dimensions windowDimensions;
    windowDimensions.width = 500;
    windowDimensions.height = 400;
    windowDimensions.startX = (screenWidth - windowDimensions.width) / 2;
    windowDimensions.startY = (screenHeight - windowDimensions.height) / 2;
    
    hWindow = CreateWindowEx(
        0,
        CLASS_NAME,
        L"Input Switcher",
        WS_OVERLAPPEDWINDOW,
        
        windowDimensions.startX, windowDimensions.startY, 
        windowDimensions.width, windowDimensions.height,

        NULL, NULL, hInstance, NULL
    );
    
    if (hWindow == NULL) {
        MessageBox(NULL, L"Couldn't create window", L"Error", MB_OK);
        return -1;
    }

    // create gui
    int margin = 20;

    RECT rect;
    if (GetClientRect(hWindow, &rect) == FALSE) {
        MessageBox(NULL, L"Couldn't get client bounds", L"Error", MB_OK);
        return -1;
    }

    int clientWidth = rect.right;
    int clientHeight = rect.bottom;

    // place buttons at the center of window
    // todo: replace letters in buttons with icons
    Dimensions buttonDimensions;
    buttonDimensions.width = 40;
    buttonDimensions.height = 40;
    buttonDimensions.startX = (clientWidth - buttonDimensions.width) / 2;
    buttonDimensions.startY = (clientHeight / 2) - margin - buttonDimensions.height;

    hButtonRight = CreateWindowExW(
        0,
        L"BUTTON", L"R",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,

        buttonDimensions.startX, buttonDimensions.startY,
        buttonDimensions.width, buttonDimensions.height,

        hWindow, NULL, hInstance, NULL
    );

    EnableWindow(hButtonRight, false);

    buttonDimensions.startY = (clientHeight / 2) + margin;

    hButtonLeft = CreateWindowExW(
        0,
        L"BUTTON", L"L",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,

        buttonDimensions.startX, buttonDimensions.startY,
        buttonDimensions.width, buttonDimensions.height,

        hWindow, NULL, hInstance, NULL
    );

    EnableWindow(hButtonLeft, false);


    int labelHeight = 20;

    // ListBox for available layouts
    // place at the left side of window
    Dimensions availableListDimensions;
    availableListDimensions.startX = margin;
    availableListDimensions.startY = margin + labelHeight;
    availableListDimensions.width = ((clientWidth - buttonDimensions.width) / 2)  - (margin * 2);
    availableListDimensions.height = clientHeight - (margin * 2) - labelHeight;

    hListBoxAvailable = CreateWindowExW(
        WS_EX_CLIENTEDGE,
        L"LISTBOX", NULL,
        WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_AUTOVSCROLL | LBS_NOTIFY,

        availableListDimensions.startX, availableListDimensions.startY,
        availableListDimensions.width, availableListDimensions.height,

        hWindow, NULL, hInstance, NULL
    );
    
    if (hListBoxAvailable == NULL) {
        MessageBox(hWindow, L"Couldn't create ListBox", L"Error", MB_OK);
        return -1;
    }

    HWND hTextAvailable = CreateWindowW(
        L"STATIC", 
        L"Available Layouts", 
        WS_CHILD | WS_VISIBLE | SS_LEFTNOWORDWRAP, 
        availableListDimensions.startX, availableListDimensions.startY - labelHeight,
        availableListDimensions.width, labelHeight,

        hWindow, NULL, hInstance, NULL
    );

    if (hTextAvailable == NULL) {
        MessageBox(hWindow, L"Couldn't create text label", L"Error", MB_OK);
        return -1;
    }

    // ListBox for active layouts
    // place at the right side of window
    Dimensions activeListDimensions;
    activeListDimensions.startX = (margin * 3) + availableListDimensions.width + buttonDimensions.width;
    activeListDimensions.startY = margin + labelHeight;
    activeListDimensions.width = ((clientWidth - buttonDimensions.width) / 2)  - (margin * 2);
    activeListDimensions.height = clientHeight - (margin * 2) - labelHeight;

    hListBoxActive = CreateWindowExW(
        WS_EX_CLIENTEDGE,
        L"LISTBOX", NULL,
        WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_AUTOVSCROLL | LBS_NOTIFY,

        activeListDimensions.startX, activeListDimensions.startY,
        activeListDimensions.width, activeListDimensions.height,

        hWindow, NULL, hInstance, NULL
    );
    
    if (hListBoxActive == NULL) {
        MessageBox(hWindow, L"Couldn't create ListBox", L"Error", MB_OK);
        return -1;
    }

    HWND hTextActive = CreateWindowW(
        L"STATIC", 
        L"Active Layouts", 
        WS_CHILD | WS_VISIBLE | SS_LEFTNOWORDWRAP, 
        activeListDimensions.startX, activeListDimensions.startY - labelHeight,
        activeListDimensions.width, labelHeight,

        hWindow, NULL, hInstance, NULL
    );

    if (hTextActive == NULL) {
        MessageBox(hWindow, L"Couldn't create text label", L"Error", MB_OK);
        return -1;
    }

    // register hot key for CapsLock
    // todo: make the key configurable
    if (!RegisterHotKey(hWindow, 0, 0, VK_CAPITAL)) {
        MessageBox(NULL, L"Couldn't regiser hot key", L"Error", MB_OK);
        return -1;
    }

    ShowWindow(hWindow, nCmdShow);
    
    // setup tray icon
    // uses default Windows information icon
    // todo: make a proper icon
    NOTIFYICONDATAW nid = {};
    nid.cbSize = sizeof(NOTIFYICONDATAW);
    nid.hWnd = hWindow;
    nid.uID = 0x10;
    nid.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE;
    nid.hIcon = LoadIcon(NULL, IDI_INFORMATION);
    
    Shell_NotifyIconW(NIM_ADD, &nid);
    
    std::vector<std::wstring> codes = getAllAvailableLayouts();
    layouts = matchLayoutData(codes);

    activeLayouts = layouts;

    if (layouts.size() <= 0) {
        MessageBox(hWindow, L"Couldn't find any keyboard layouts", L"Warning", MB_OK);
    }

    // fill list with names of layouts
    for (int i = 0; i < layouts.size(); i++) {
        int pos = (int) SendMessage(hListBoxActive, LB_ADDSTRING, 0, (LPARAM) layouts[i].layoutName.c_str());

        SendMessage(hListBoxActive, LB_SETITEMDATA, pos, (LPARAM) &(layouts[i].layoutCode));
    }
    
    MSG msg = {};
    while(GetMessage(&msg, NULL, 0, 0) > 0) {
        if (msg.message == WM_HOTKEY) {
            onHotKey();
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
    if (uMsg == WM_DESTROY) {
        PostQuitMessage(0);
        return 0;
    } else if (uMsg == WM_PAINT) {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        FillRect(hdc, &ps.rcPaint, HBRUSH(COLOR_WINDOW));
        
        EndPaint(hwnd, &ps);
        
        return 0;
    } else if (uMsg == WM_COMMAND) {
        if (lParam != 0) {
            HWND windowHandle = HWND(lParam);
            int command = HIWORD(wParam);

            if (onCommand(windowHandle, command)) {
                return 0;
            }
        }
    } 
    
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

std::vector<std::wstring> getAllAvailableLayouts()
{
    int size = GetKeyboardLayoutList(0, nullptr);
    HKL list[size];

    std::vector<std::wstring> codes;
    
    if (GetKeyboardLayoutList(size, list) <= 0) {
        MessageBox(hWindow, L"Failed to query available keyboard layouts", L"Error", MB_OK);
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
        MessageBox(hWindow, L"Error while attetmpting to open registry", L"Error", MB_OK);
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
            MessageBox(hWindow, L"Failed to read value", L"Error", MB_OK);
            info.layoutName = std::wstring(L"Unknown Layout ") + info.layoutCode;
        } else {
            info.layoutName = std::wstring(data);
        }

        matched.push_back(info);
    }

    return matched;
}

void onHotKey()
{
    if (activeLayouts.size() <= 0) {
        return;
    }

    HWND current = GetForegroundWindow();
    
    wchar_t code[8];
    GetKeyboardLayoutNameW(code);
    
    HKL hkl;

    // find current layout
    int index = 0;
    for (int i = 0; i != activeLayouts.size(); i++) {
        if (activeLayouts[i].layoutCode == code) {
            index = i + 1;
            break;
        }
    }

    if (index >= activeLayouts.size()) {
        index = 0;
    }

    // change to next layout
    hkl = LoadKeyboardLayout(activeLayouts[index].layoutCode.c_str(), KLF_ACTIVATE);
    
    PostMessageW(
        current,
        WM_INPUTLANGCHANGEREQUEST,
        0,
        (LPARAM) hkl
    );
}

bool onCommand(HWND windowHandle, int command)
{
    bool isListBox = windowHandle == hListBoxAvailable || windowHandle == hListBoxActive;
    bool isDirectionalButton = windowHandle == hButtonRight || windowHandle == hButtonLeft;

    if (isListBox || isDirectionalButton) {
        HWND lb1 = hListBoxAvailable;
        HWND lb2 = hListBoxActive;

        HWND btn1 = hButtonRight;
        HWND btn2 = hButtonLeft;

        if (windowHandle == hListBoxActive || windowHandle == hButtonLeft) {
            std::swap(lb1, lb2);
            std::swap(btn1, btn2);
        }

        if (isListBox) {
            // enable respective button if an item in listbox was selected
            if (command == LBN_SELCHANGE) {
                LRESULT data = SendMessageW(lb1, LB_GETCURSEL, 0, 0);
                
                SendMessageW(lb2, LB_SETCURSEL, -1, 0);

                EnableWindow(btn2, false);
                EnableWindow(btn1, data != LB_ERR);
            } else if (command == LBN_SELCANCEL) {
                EnableWindow(btn1, false);
            }
        } else if (isDirectionalButton && command == BN_CLICKED) {
            // get layout name and code from selected listbox item
            int index = SendMessageW(lb1, LB_GETCURSEL, 0, 0);
            int length = SendMessageW(lb1, LB_GETTEXTLEN, index, 0);
            wchar_t text[length];

            SendMessageW(lb1, LB_GETTEXT, index, LPARAM(LPTSTR(text)));
            LRESULT data = SendMessageW(lb1, LB_GETITEMDATA, index, 0);
            
            if (data == LB_ERR) {
                MessageBoxW(hWindow, L"Error while trying to get item data", L"Error", MB_OK);
            } else {
                // move item to another listbox
                std::wstring code = *((std::wstring*) data);

                int pos = SendMessageW(lb2, LB_ADDSTRING, 0, (LPARAM) text);
                SendMessageW(lb2, LB_SETITEMDATA, pos, (LPARAM) data);

                SendMessageW(lb1, LB_DELETESTRING, index, 0);
                SendMessageW(lb1, LB_SETCURSEL, -1, 0);

                EnableWindow(btn1, false);
                EnableWindow(btn2, false);

                refreshActiveList();
            }
        }

        return true;
    }

    return false;
}

void refreshActiveList()
{
    int count = SendMessageW(hListBoxActive, LB_GETCOUNT, 0, 0);

    std::vector<LayoutInfo> newActiveList;

    for (int i = 0; i < count; i++) {
        int length = SendMessageW(hListBoxActive, LB_GETTEXTLEN, i, 0);
        wchar_t text[length];

        SendMessageW(hListBoxActive, LB_GETTEXT, i, LPARAM(LPTSTR(text)));
        LRESULT data = SendMessageW(hListBoxActive, LB_GETITEMDATA, i, 0);
        std::wstring code = *((std::wstring*) data);

        LayoutInfo info;
        info.layoutName = std::wstring(text);
        info.layoutCode = code;

        newActiveList.push_back(info);
    }

    activeLayouts = newActiveList;
}