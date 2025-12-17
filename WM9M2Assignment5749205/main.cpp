#define WIDTH 1920   // Dell XPS - 1920 || HP Omen Max 16 - 2560
#define HEIGHT 1200  // Dell XPS - 1200 || HP Omen Max 16 - 1600

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
	camera.setCamera(WIDTH, HEIGHT, 10000.f, 0.01f, 45.f);

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

		time += dt;
		camera.updateViewMatrix();
		Matrix v = camera.view;
		Matrix p = camera.projection;
		Matrix vp = v * p;

		character.movePlayer(&camera, &window, dt);
		character.inspectWeapon(&window);
		character.meleeAttack(&window);
		character.selectWeapon(&window);
		character.putawayWeapon(&window);
		character.reload(&window);
		character.toggleAlternateFireMode(&window);
		character.shoot(&window);

		// TO:DO - Camera Control via Mouse (Arrow Keys for now)
		if (window.keys[VK_UP]) camera.pitch(1.f * dt);
		if (window.keys[VK_DOWN]) camera.pitch(-1.f * dt);
		if (window.keys[VK_LEFT]) camera.rotateY(1.f * dt);
		if (window.keys[VK_RIGHT]) camera.rotateY(-1.f * dt);

		Matrix planeWorld = Matrix::identity() * Matrix::rotateOnYAxis(M_PI);
		Matrix cubeWorld = Matrix::translate(Vec3(-5.f, 0.f, 0.f)) * Matrix::rotateOnYAxis(M_PI);
		Matrix sphereWorld = Matrix::identity();
		Matrix acaciaWorld = Matrix::scale(Vec3(0.02f, 0.02f, 0.02f)) * Matrix::translate(Vec3(-5.f, 1.f, 0.f)) * Matrix::rotateOnYAxis(M_PI);
		Matrix foodWarmerWorld = Matrix::scale(Vec3(2.f, 2.f, 2.f)) * Matrix::translate(Vec3(0.f, 0.f, 3.f)) * Matrix::rotateOnYAxis(M_PI);
		Matrix trexWorld = Matrix::scale(Vec3(0.01f, 0.01f, 0.01f)) * Matrix::rotateOnYAxis(M_PI);
		Matrix characterWorld = Matrix::scale(Vec3(0.3f, 0.3f, 0.3f)) * Matrix::rotateOnYAxis(M_PI) * Matrix::translate(Vec3(0.f, 1.f, 0.f)) * camera.view.invert();

		core.beginRenderPass();

		cube.draw(&core, &psos, &shaders, vp, cubeWorld);
		plane.draw(&core, &psos, &shaders, vp, planeWorld);
		acacia.draw(&core, &psos, &textures, &shaders, vp, acaciaWorld);
		foodWarmer.draw(&core, &psos, &textures, &shaders, vp, foodWarmerWorld);

		cubeWorld = Matrix::translate(Vec3(5.f, 0.f, 0.f)) * Matrix::rotateOnYAxis(M_PI);
		cube.draw(&core, &psos, &shaders, vp, cubeWorld);
		
		animatedInstance.update("run", dt);
		if (animatedInstance.animationFinished() == true) animatedInstance.resetAnimationTime();
		trex.draw(&core, &animatedInstance, &textures, &psos, &shaders, vp, trexWorld);
		
		character.animate(dt);
		character.draw(&core, &textures, &psos, &shaders, vp, characterWorld);
		sphere.draw(&core, &psos, &textures, &shaders, vp, sphereWorld);

		core.finishFrame();
	}
	core.flushGraphicsQueue();
	FreeConsole();
	return 0;
}