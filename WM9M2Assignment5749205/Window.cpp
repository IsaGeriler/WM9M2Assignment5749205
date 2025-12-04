#include "Window.h"

Window* window;

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
		case WM_DESTROY: {
			PostQuitMessage(0);
			exit(0);
			return 0;
		}
		case WM_CLOSE: {
			PostQuitMessage(0);
			exit(0);
			return 0;
		}
		case WM_KEYDOWN: {
			window->keys[(unsigned int)wParam] = true;
			return 0;
		}
		case WM_KEYUP: {
			window->keys[(unsigned int)wParam] = false;
			return 0;
		}
		case WM_LBUTTONDOWN: {
			window->updateMouse(WINDOW_GET_X_LPARAM(lParam), WINDOW_GET_Y_LPARAM(lParam));
			window->mouseButtons[0] = true;
			return 0;
		}
		case WM_LBUTTONUP: {
			window->updateMouse(WINDOW_GET_X_LPARAM(lParam), WINDOW_GET_Y_LPARAM(lParam));
			window->mouseButtons[0] = false;
			return 0;
		}
		case WM_MBUTTONDOWN: {
			window->updateMouse(WINDOW_GET_X_LPARAM(lParam), WINDOW_GET_Y_LPARAM(lParam));
			window->mouseButtons[1] = true;
			return 0;
		}
		case WM_MBUTTONUP: {
			window->updateMouse(WINDOW_GET_X_LPARAM(lParam), WINDOW_GET_Y_LPARAM(lParam));
			window->mouseButtons[1] = false;
			return 0;
		}
		case WM_RBUTTONDOWN: {
			window->updateMouse(WINDOW_GET_X_LPARAM(lParam), WINDOW_GET_Y_LPARAM(lParam));
			window->mouseButtons[2] = true;
			return 0;
		}
		case WM_RBUTTONUP: {
			window->updateMouse(WINDOW_GET_X_LPARAM(lParam), WINDOW_GET_Y_LPARAM(lParam));
			window->mouseButtons[2] = false;
			return 0;
		}
		case WM_MOUSEWHEEL: {
			window->updateMouse(WINDOW_GET_X_LPARAM(lParam), WINDOW_GET_Y_LPARAM(lParam));
			window->mouseWheel += GET_WHEEL_DELTA_WPARAM(wParam);
			return 0;
		}
		case WM_MOUSEMOVE: {
			window->updateMouse(WINDOW_GET_X_LPARAM(lParam), WINDOW_GET_Y_LPARAM(lParam));
			return 0;
		}
		default: {
			return DefWindowProc(hwnd, msg, wParam, lParam);
		}
	}
}

Window::~Window() {
	ShowCursor(true);
	ClipCursor(NULL);
}

void Window::initialize(int _width, int _height, std::string _name, float _zoom, bool window_fullscreen, int window_x, int window_y) {
	WNDCLASSEX wc;
	hinstance = GetModuleHandle(NULL);
	name = _name;
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hinstance;
	wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
	wc.hIconSm = wc.hIcon;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.lpszMenuName = NULL;
	std::wstring wname = std::wstring(name.begin(), name.end());
	wc.lpszClassName = wname.c_str();
	wc.cbSize = sizeof(WNDCLASSEX);
	RegisterClassEx(&wc);
	DWORD style;

	// window_fullscreen
	if (0) {
		width = GetSystemMetrics(SM_CXSCREEN);
		height = GetSystemMetrics(SM_CYSCREEN);
		DEVMODE fs;
		memset(&fs, 0, sizeof(DEVMODE));
		fs.dmSize = sizeof(DEVMODE);
		fs.dmPelsWidth = (unsigned long)width;
		fs.dmPelsHeight = (unsigned long)height;
		fs.dmBitsPerPel = 32;
		fs.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
		ChangeDisplaySettings(&fs, CDS_FULLSCREEN);
		style = WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP;
	} else {
		width = _width;
		height = _height;
		style = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
	}

	RECT wr = { 0, 0, width * _zoom, height * _zoom };
	AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);
	hwnd = CreateWindowEx(WS_EX_APPWINDOW, wname.c_str(), wname.c_str(), style, window_x, window_y, wr.right - wr.left, wr.bottom - wr.top, NULL, NULL, hinstance, this);
	invZoom = 1.0f / (float)_zoom;
	ShowWindow(hwnd, SW_SHOW);
	SetForegroundWindow(hwnd);
	SetFocus(hwnd);
	useMouseClip = false;
	ShowCursor(true);
	window = this;
}

void Window::updateMouse(int _mousex, int _mousey) {
	mousex += _mousex;
	mousey += _mousey;
}

void Window::processMessages() {
	MSG msg;
	ZeroMemory(&msg, sizeof(MSG));
	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

void Window::clipMouseToWindow() {
	RECT rect;
	GetClientRect(hwnd, &rect);

	POINT ul;
	ul.x = rect.left;
	ul.y = rect.top;
	
	POINT lr;
	lr.x = rect.right;
	lr.y = rect.bottom;
	
	MapWindowPoints(hwnd, nullptr, &ul, 1);
	MapWindowPoints(hwnd, nullptr, &lr, 1);
	
	rect.left = ul.x;
	rect.top = ul.y;
	rect.right = lr.x;
	rect.bottom = lr.y;
	
	ClipCursor(&rect);
}

void Window::checkInput() {
	if (useMouseClip) clipMouseToWindow();
	processMessages();
}

bool Window::keyPressed(int key) { return keys[key]; }

int Window::getMouseInWindowX() {
	POINT p;
	GetCursorPos(&p);
	ScreenToClient(hwnd, &p);
	
	RECT rect;
	GetClientRect(hwnd, &rect);
	
	p.x = p.x - rect.left;
	p.x = p.x * invZoom;
	
	return p.x;
}
int Window::getMouseInWindowY() {
	POINT p;
	GetCursorPos(&p);
	ScreenToClient(hwnd, &p);
	
	RECT rect;
	GetClientRect(hwnd, &rect);
	
	p.y = p.y - rect.top;
	p.y = p.y * invZoom;
	
	return p.y;
}