#include "Window.h"

const int WIDTH = 1024;
const int HEIGHT = 1024;

extern "C" {
	_declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
}

int WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ PSTR lpCmdLine, _In_ int nCmdShow) {
	Window window;
	window.initialize(WIDTH, HEIGHT, "My Window");

	while (true) {
		if (window.keyPressed(VK_ESCAPE)) break;
		window.processMessages();
	}
	return 0;
}