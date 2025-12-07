#define WIDTH 1920
#define HEIGHT 1080

#include "Core.h"
#include "Cube.h"
#include "Mesh.h"
#include "Plane.h"
#include "PSOManager.h"
#include "Shaders.h"
#include "Sphere.h"
#include "StaticModel.h"
#include "AnimatedModel.h"
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

	Sphere sphere;
	sphere.initialize(&core, &psos, &shaders, 32, 32, 500.f);

	StaticModel acacia;
	acacia.load(&core, &psos, &shaders, "Assets/acacia_003.gem");

	AnimatedModel trex;
	trex.load(&core, &psos, &shaders, "Assets/TRex.gem");
	AnimationInstance animatedInstance;
	animatedInstance.initialize(&trex.animation, 0);
	
	Timer timer;
	float time = 0.f;

	while (true) {
		core.beginFrame();
		float dt = timer.dt();
		window.processMessages();
		if (window.keys[VK_ESCAPE] == 1) break;

		time += dt;
		Vec3 from = Vec3(11.f * cos(time), 5.f, 11.f * sinf(time));
		Matrix v = Matrix::lookAt(from, Vec3(0.f, 0.f, 0.f), Vec3(0.f, 1.f, 0.f));
		Matrix p = Matrix::projection(WIDTH, HEIGHT, 10000.f, 0.01, 60.f);  // Projection/Perspective Matrix
		Matrix vp = v * p;

		Matrix planeWorld = Matrix::identity();
		Matrix cubeWorld = Matrix::translate(Vec3(-5.f, 0.f, 0.f));
		Matrix sphereWorld = Matrix::identity();
		Matrix acaciaWorld = Matrix::scale(Vec3(0.002f, 0.002f, 0.002f));
		Matrix trexWorld = Matrix::scale(Vec3(0.01f, 0.01f, 0.01f));

		core.beginRenderPass();
		
		animatedInstance.update("run", dt);
		if (animatedInstance.animationFinished() == true)
			animatedInstance.resetAnimationTime();
		trex.draw(&core, &animatedInstance, &psos, &shaders, vp, trexWorld);

		cube.draw(&core, &psos, &shaders, vp, cubeWorld);
		plane.draw(&core, &psos, &shaders, vp, planeWorld);
		sphere.draw(&core, &psos, &shaders, vp, sphereWorld);
		acacia.draw(&core, &psos, &shaders, vp, acaciaWorld);

		cubeWorld = Matrix::translate(Vec3(5.f, 0.f, 0.f));
		cube.draw(&core, &psos, &shaders, vp, cubeWorld);
		
		core.finishFrame();
	}
	core.flushGraphicsQueue();
	return 0;
}