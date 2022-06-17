#ifndef UNICODE
#define UNICODE
#endif

#include <windows.h>
#include <string>

std::wstring to_wide (const std::string &multi) {
    std::wstring wide; wchar_t w; mbstate_t mb {};
    size_t n = 0, len = multi.length () + 1;
    while (auto res = mbrtowc (&w, multi.c_str () + n, len - n, &mb)) {
        if (res == size_t (-1) || res == size_t (-2))
            throw "invalid encoding";

        n += res;
        wide += w;
    }
    return wide;
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
	
	HWND hwnd = CreateWindowEx(
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
		return -1;
	}
	
	if (!RegisterHotKey(hwnd, 0, 0, VK_CAPITAL)) {
		MessageBox(NULL, L"Couldn't regiser hot key", L"Error", MB_OK);
	}
	
	ShowWindow(hwnd, nCmdShow);
	
	NOTIFYICONDATAW nid = {};
	nid.cbSize = sizeof(NOTIFYICONDATAW);
	nid.hWnd = hwnd;
	nid.uID = 0x10;
	nid.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE;
	nid.hIcon = LoadIcon(NULL, IDI_INFORMATION);
	
	Shell_NotifyIconW(NIM_ADD, &nid);
	
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
			
			//SendNotifyMessageW(
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