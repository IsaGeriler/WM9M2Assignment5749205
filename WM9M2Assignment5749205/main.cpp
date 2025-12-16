#define WIDTH 2560   // Dell XPS - 1920 || HP Omen Max 16 - 2560
#define HEIGHT 1600  // Dell XPS - 1200 || HP Omen Max 16 - 1600

#include "Camera.h"
#include "Core.h"
#include "Character.h"
#include "Cube.h"
#include "Mesh.h"
#include "Plane.h"
#include "PSOManager.h"
#include "Shaders.h"
#include "Sphere.h"
#include "StaticModel.h"
#include "AnimatedModel.h"
#include "Texture.h"
#include "Timer.h"
#include "Window.h"

extern "C" {
	_declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
}

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ PSTR lpCmdLine, _In_ int nCmdShow) {
	// Bring back console output
	AllocConsole();								 // Allocate console
	FILE* stream;
	freopen_s(&stream, "CONOUT$", "w", stdout);  // Redirect stdout to console
	printf("Welcome back, console!\n");			 // Test output printing
	
	Window window;
	window.initialize(WIDTH, HEIGHT, "WM9M2: Post Module Assignment (5749205)");

	Core core;
	core.initialize(window.hwnd, WIDTH, HEIGHT);

	Camera camera;
	camera.setLense(WIDTH, HEIGHT, 10000.f, 0.01f, 45.f);

	PSOManager psos;
	ShaderManager shaders;
	TextureManager textures;
	
	Plane plane;
	plane.initialize(&core, &psos, &shaders);

	Cube cube;
	cube.initialize(&core, &psos, &shaders);

	Sphere sphere;
	sphere.initialize(&core, &psos, &textures, &shaders, 32, 32, 100.f);

	StaticModel acacia;
	acacia.load(&core, &psos, &textures, &shaders, "Models/banana2_LOD5.gem");

	StaticModel foodWarmer;
	foodWarmer.load(&core, &psos, &textures, &shaders, "Models/Food_Warmer_07a.gem");

	AnimatedModel trex;
	trex.load(&core, &psos, &textures, &shaders, "Models/TRex.gem");
	AnimationInstance animatedInstance;
	animatedInstance.initialize(&trex.animation, 0);

	Character character;
	character.initialize(&core, &psos, &textures, &shaders, "Models/AutomaticCarbine.gem");
	
	Timer timer;
	float time = 0.f;

	while (true) {
		core.beginFrame();
		float dt = timer.dt();
		window.processMessages();
		if (window.keys[VK_ESCAPE] == 1) break;

		// Controlling the camera with keyboard (mouse support soon...)
		if (window.keys['W'] == 1) camera.walk(10.f * dt);
		if (window.keys['S'] == 1) camera.walk(-10.f * dt);
		if (window.keys['A'] == 1) camera.strafe(-10.f * dt);
		if (window.keys['D'] == 1) camera.strafe(10.f * dt);

		if (window.keys[VK_UP]) camera.pitch(1.f * dt);
		if (window.keys[VK_LEFT]) camera.rotateY(-1.f * dt);
		if (window.keys[VK_DOWN]) camera.pitch(-1.f * dt);
		if (window.keys[VK_RIGHT]) camera.rotateY(1.f * dt);

		camera.updateViewMatrix();

		time += dt;
		Matrix v = camera.view;
		Matrix p = camera.projection;
		Matrix vp = v * p;

		Matrix planeWorld = Matrix::identity();
		Matrix cubeWorld = Matrix::translate(Vec3(-5.f, 0.f, 0.f));
		Matrix sphereWorld = Matrix::identity();
		Matrix acaciaWorld = Matrix::scale(Vec3(0.02f, 0.02f, 0.02f)) * Matrix::translate(Vec3(-5.f, 1.f, 0.f));
		Matrix foodWarmerWorld = Matrix::scale(Vec3(2.f, 2.f, 2.f)) * Matrix::translate(Vec3(0.f, 0.f, 3.f));
		Matrix trexWorld = Matrix::scale(Vec3(0.01f, 0.01f, 0.01f));
		Matrix characterWorld = Matrix::scale(Vec3(0.25f, 0.25f, 0.25f)) * Matrix::translate(Vec3(5.f, 0.f, 5.f));

		core.beginRenderPass();

		cube.draw(&core, &psos, &shaders, vp, cubeWorld);
		plane.draw(&core, &psos, &shaders, vp, planeWorld);
		acacia.draw(&core, &psos, &textures, &shaders, vp, acaciaWorld);
		foodWarmer.draw(&core, &psos, &textures, &shaders, vp, foodWarmerWorld);

		cubeWorld = Matrix::translate(Vec3(5.f, 0.f, 0.f));
		cube.draw(&core, &psos, &shaders, vp, cubeWorld);
		
		animatedInstance.update("run", dt);
		if (animatedInstance.animationFinished() == true) animatedInstance.resetAnimationTime();
		trex.draw(&core, &animatedInstance, &textures, &psos, &shaders, vp, trexWorld);

		character.setAnmationState(Run);
		character.updateAnimation(dt);
		character.resetAnimationTime();
		character.draw(&core, &textures, &psos, &shaders, vp, characterWorld);

		sphere.draw(&core, &psos, &textures, &shaders, vp, sphereWorld);

		core.finishFrame();
	}
	core.flushGraphicsQueue();
	FreeConsole();
	return 0;
}