#pragma once

#define NOMINMAX
#define WINDOW_GET_X_LPARAM(lp) ((int)(short)LOWORD(lp))
#define WINDOW_GET_Y_LPARAM(lp) ((int)(short)HIWORD(lp))

#include <string>
#include <Windows.h>

class Window {
public:
	HWND hwnd;			   // Handle to a window
	HINSTANCE hinstance;   // Handle to an instance

	std::string name;	   // Window Name
	int width, height;	   // Window width and height
	
	bool keys[256];		   // Store keys
	bool mouseButtons[3];  // Left - Middle - Right
	int mousex, mousey;    // Store mouse coordinates

	void initialize(int _width, int _height, std::string _name, int window_x = 0, int window_y = 0);
	void updateMouse(int _mousex, int _mousey);
	void processMessages();
};