#include "Core.h"
#include "Window.h"

const int WIDTH = 1024;
const int HEIGHT = 1024;

extern "C" {
	_declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
}

int WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ PSTR lpCmdLine, _In_ int nCmdShow) {
	Window window;
	Core core;

	window.initialize(WIDTH, HEIGHT, "My Window");
	core.initialize(window.hwnd, window.width, window.height);

	while (true) {
		core.beginFrame();
		window.processMessages();
		if (window.keys[VK_ESCAPE] == 1) break;
		core.finishFrame();
	}
	core.flushGraphicsQueue();
	return 0;
}