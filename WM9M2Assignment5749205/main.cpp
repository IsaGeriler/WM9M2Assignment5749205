#define WIDTH 1024
#define HEIGHT 1024

#include "Core.h"
#include "Cube.h"
#include "Mesh.h"
#include "Plane.h"
#include "PSOManager.h"
#include "Shaders.h"
#include "Sphere.h"
#include "Timer.h"
#include "Window.h"

extern "C" {
	_declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
}

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ PSTR lpCmdLine, _In_ int nCmdShow) {
	
	Window window;
	window.initialize(WIDTH, HEIGHT, "WM9M2: Post Module Assignment (5749205)");

	Core core;
	core.initialize(window.hwnd, WIDTH, HEIGHT);

	PSOManager psos;
	ShaderManager shaders;
	
	Plane plane;
	plane.initialize(&core, &psos, &shaders);

	Cube cube;
	cube.initialize(&core, &psos, &shaders);

	Sphere skybox;
	skybox.initialize(&core, &psos, &shaders, 64, 64, 100.f);
	
	Timer timer;
	float time = 0.f;

	while (true) {
		core.beginFrame();
		float dt = timer.dt();
		window.processMessages();
		if (window.keys[VK_ESCAPE] == 1) break;

		time += dt;
		Vec3 from = Vec3(11.f * cos(time), 5.f, 11.f * sinf(time));
		Matrix v = Matrix::lookAt(from, Vec3(0.f, 1.f, 0.f), Vec3(0.f, 1.f, 0.f));
		Matrix p = Matrix::projection(WIDTH, HEIGHT, 10000.f, 0.01, 60.f);  // Projection (Perspective) Matrix
		Matrix vp = p.mul(v);

		shaders.updateConstantVertexShaderBuffer("Plane", "staticMeshBuffer", "VP", &vp);
		shaders.updateConstantVertexShaderBuffer("Cube", "staticMeshBuffer", "VP", &vp);
		shaders.updateConstantVertexShaderBuffer("Sphere", "staticMeshBuffer", "VP", &vp);

		core.beginRenderPass();
		plane.draw(&core, &psos, &shaders);
		cube.draw(&core, &psos, &shaders);
		skybox.draw(&core, &psos, &shaders);
		core.finishFrame();
	}
	core.flushGraphicsQueue();
	return 0;
}