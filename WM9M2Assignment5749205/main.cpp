#define WIDTH 1920   // Dell XPS - 1920 || HP Omen Max 16 - 2560
#define HEIGHT 1200  // Dell XPS - 1200 || HP Omen Max 16 - 1600

#include "Camera.h"
#include "Core.h"
#include "Collision.h"
#include "Character.h"
#include "Cube.h"
#include "NPC.h"
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
	acacia.load(&core, &psos, &textures, &shaders, "Models/banana2.gem");

	StaticInstancedModel instancedTree;
	StaticInstancedVertexAnimModel instancedGrass;

	std::vector<INSTANCE_DATA> instances;
	std::vector<INSTANCE_DATA> instancesGrass;
	
	for (int i = 1; i <= 24; i+=3) {
		for (int j = 1; j <= 24; j+=3) {
			INSTANCE_DATA insQ1;
			INSTANCE_DATA insQ2;
			INSTANCE_DATA insQ3;
			INSTANCE_DATA insQ4;
			
			insQ1.world = Matrix::translate(Vec3((float)i * 100.f, -65.f, (float)j * 100.f)) * Matrix::scale(Vec3(0.03f, 0.03f, 0.03f)) * Matrix::rotateOnYAxis(M_PI);
			instances.push_back(insQ1);

			insQ2.world = Matrix::translate(Vec3(-(float)i * 100.f, -65.f, (float)j * 100.f)) * Matrix::scale(Vec3(0.03f, 0.03f, 0.03f)) * Matrix::rotateOnYAxis(M_PI);
			instances.push_back(insQ2);

			insQ3.world = Matrix::translate(Vec3((float)i * 100.f, -65.f, -(float)j * 100.f)) * Matrix::scale(Vec3(0.03f, 0.03f, 0.03f)) * Matrix::rotateOnYAxis(M_PI);
			instances.push_back(insQ3);

			insQ4.world = Matrix::translate(Vec3(-(float)i * 100.f, -65.f, -(float)j * 100.f)) * Matrix::scale(Vec3(0.03f, 0.03f, 0.03f)) * Matrix::rotateOnYAxis(M_PI);
			instances.push_back(insQ4);
		}
	}

	AABB bananaTreeAABB;
	std::vector<AABB> bananaTreeAABBs;
	bananaTreeAABB.min = Vec3(-100.f, -300.f, -100.f);
	bananaTreeAABB.max = Vec3(100.f, 300.f, 100.f);

	for (int i = 0; i < instances.size(); i++) {
		AABB tree;
		Vec3 treeCorners[8] = {
			Vec3(bananaTreeAABB.min.x, bananaTreeAABB.min.y, bananaTreeAABB.min.z),
			Vec3(bananaTreeAABB.max.x, bananaTreeAABB.min.y, bananaTreeAABB.min.z),
			Vec3(bananaTreeAABB.min.x, bananaTreeAABB.max.y, bananaTreeAABB.min.z),
			Vec3(bananaTreeAABB.min.x, bananaTreeAABB.min.y, bananaTreeAABB.max.z),
			Vec3(bananaTreeAABB.max.x, bananaTreeAABB.max.y, bananaTreeAABB.min.z),
			Vec3(bananaTreeAABB.max.x, bananaTreeAABB.min.y, bananaTreeAABB.max.z),
			Vec3(bananaTreeAABB.min.x, bananaTreeAABB.max.y, bananaTreeAABB.max.z),
			Vec3(bananaTreeAABB.max.x, bananaTreeAABB.max.y, bananaTreeAABB.max.z)
		};

		for (int j = 0; j < 8; j++) {
			Vec3 aabbWorld = instances[i].world.mulPoint(treeCorners[j]);
			tree.extend(aabbWorld);
		}
		bananaTreeAABBs.push_back(tree);
	}

	for (int i = 1; i <= 136; i++) {
		for (int j = 1; j <= 136; j++) {
			if (i * 100 % 200 == 0 || j * 100 % 200) continue;
			INSTANCE_DATA insGrassQ1;
			INSTANCE_DATA insGrassQ2;
			INSTANCE_DATA insGrassQ3;
			INSTANCE_DATA insGrassQ4;
			
			insGrassQ1.world = Matrix::translate(Vec3((float)i * 50.f, -300.f, (float)j * 50.f)) * Matrix::scale(Vec3(0.01f, 0.01f, 0.01f)) * Matrix::rotateOnYAxis(M_PI);
			instancesGrass.push_back(insGrassQ1);
			
			insGrassQ2.world = Matrix::translate(Vec3(-(float)i * 50.f, -300.f, (float)j * 50.f)) * Matrix::scale(Vec3(0.01f, 0.01f, 0.01f)) * Matrix::rotateOnYAxis(M_PI);
			instancesGrass.push_back(insGrassQ2);

			insGrassQ3.world = Matrix::translate(Vec3((float)i * 50.f, -300.f, -(float)j * 50.f)) * Matrix::scale(Vec3(0.01f, 0.01f, 0.01f)) * Matrix::rotateOnYAxis(M_PI);
			instancesGrass.push_back(insGrassQ3);

			insGrassQ4.world = Matrix::translate(Vec3(-(float)i * 50.f, -300.f, -(float)j * 50.f)) * Matrix::scale(Vec3(0.01f, 0.01f, 0.01f)) * Matrix::rotateOnYAxis(M_PI);
			instancesGrass.push_back(insGrassQ4);
		}
	}

	instancedTree.load(&core, &psos, &textures, &shaders, "Models/banana2.gem", instances);
	instancedGrass.load(&core, &psos, &textures, &shaders, "Models/grass_008.gem", instancesGrass);

	StaticModel truck;
	truck.load(&core, &psos, &textures, &shaders, "Models/Truck_02b.gem");

	AnimatedModel trex;
	trex.load(&core, &psos, &textures, &shaders, "Models/TRex.gem");
	AnimationInstance animatedInstance;
	animatedInstance.initialize(&trex.animation, 0);

	Character character;
	character.initialize(&core, &psos, &textures, &shaders, "Models/AutomaticCarbine.gem");
	
	NPC npc;
	npc.initialize(&core, &psos, &textures, &shaders, "Models/TRex.gem");

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

		// Camera Control via Arrow Keys
		if (window.keys[VK_UP]) camera.pitch(0.8f * dt);
		if (window.keys[VK_DOWN]) camera.pitch(-0.8f * dt);
		if (window.keys[VK_LEFT]) camera.rotateY(0.8f * dt);
		if (window.keys[VK_RIGHT]) camera.rotateY(-0.8f * dt);

		// Player Control
		character.movePlayer(&camera, &window, dt);
		if (window.keys['I'] == 1) character.inspectWeapon();
		if (window.keys['M'] == 1) character.meleeAttack();
		if (window.keys['P'] == 1) character.putawayWeapon();
		if (window.keys['O'] == 1) character.selectWeapon();
		if (window.keys['R'] == 1) character.reload();
		if (window.keys[VK_SPACE] == 1) character.toggleAlternateFireMode();
		if (window.keys['F'] == 1) character.shoot();
		character.animate(dt);

		Matrix planeWorld = Matrix::identity();
		Matrix cubeWorld = Matrix::translate(Vec3(-5.f, 0.f, 0.f)) * Matrix::rotateOnYAxis(M_PI);
		Matrix sphereWorld = Matrix::identity() * Matrix::rotateOnYAxis(M_PI);
		Matrix acaciaWorld = Matrix::scale(Vec3(0.02f, 0.02f, 0.02f)) * Matrix::translate(Vec3(-5.f, 1.f, 0.f)) * Matrix::rotateOnYAxis(M_PI);
		Matrix truckWorld = Matrix::translate(Vec3(-10.f, -2.f, 0.f)) * Matrix::rotateOnYAxis(M_PI / 4);
		Matrix trexWorld = Matrix::translate(Vec3(0.f, -160.f, -2000.f)) * Matrix::scale(Vec3(0.01f, 0.01f, 0.01f)) * Matrix::rotateOnYAxis(M_PI);
		Matrix trexWorld2 = Matrix::translate(Vec3(110.f, -160.f, -1000.f)) * Matrix::scale(Vec3(0.01f, 0.01f, 0.01f)) * Matrix::rotateOnYAxis(M_PI);
		Matrix characterWorld = Matrix::scale(Vec3(0.3f, 0.3f, 0.3f)) * Matrix::rotateOnYAxis(M_PI) * Matrix::translate(Vec3(0.f, 1.f, 0.f)) * camera.view.invert();

		core.beginRenderPass();

		//cube.draw(&core, &psos, &shaders, vp, cubeWorld);
		plane.draw(&core, &psos, &shaders, vp, planeWorld);
		acacia.draw(&core, &psos, &textures, &shaders, vp, acaciaWorld);
		truck.draw(&core, &psos, &textures, &shaders, vp, truckWorld);

		cubeWorld = Matrix::translate(Vec3(5.f, 0.f, 0.f)) * Matrix::rotateOnYAxis(M_PI);
		//cube.draw(&core, &psos, &shaders, vp, cubeWorld);
		
		animatedInstance.update("run", dt);
		if (animatedInstance.animationFinished() == true) animatedInstance.resetAnimationTime();
		trex.draw(&core, &animatedInstance, &textures, &psos, &shaders, vp, trexWorld);

		npc.animate(dt);
		npc.draw(&core, &textures, &psos, &shaders, vp, trexWorld2);
		
		character.draw(&core, &textures, &psos, &shaders, vp, characterWorld);
		instancedTree.draw(&core, &psos, &textures, &shaders, vp);
		instancedGrass.draw(&core, &psos, &textures, &shaders, vp, time);
		sphere.draw(&core, &psos, &textures, &shaders, vp, sphereWorld);

		core.finishFrame();
	}
	core.flushGraphicsQueue();
	FreeConsole();
	return 0;
}