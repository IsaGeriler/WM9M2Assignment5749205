#pragma once

#include <string>

#include "AnimatedModel.h"
#include "Camera.h"
#include "Core.h"
#include "MyMath.h"
#include "Window.h"

// Animated Carbine
enum State {
	Pose, Select, Putaway, EmptySelect, Idle,
	Inspect, Walk, Run, Fire, AlternateFire,
	MeleeAttack, ZoomIdle, ZoomWalk, ZoomFire,
	ZoomAlternateFire, AlternateFireModeOn,
	DryFire, Reload, EmptyReload
};

std::string getAnimationByState(State state) {
	switch (state) {
		case Pose: return "00 pose";
		case Select: return "01 select";
		case Putaway: return "02 putaway";
		case EmptySelect: return "03 empty select";
		case Idle: return "04 idle";
		case Inspect: return "05 inspect";
		case Walk: return "06 walk";
		case Run: return "07 run";
		case Fire: return "08 fire";
		case AlternateFire: return "09 alternate fire";
		case MeleeAttack: return "10 melee attack";
		case ZoomIdle: return "11 zoom idle";
		case ZoomWalk: return "12 zoom walk";
		case ZoomFire: return "13 zoom fire";
		case ZoomAlternateFire: return "14 zoom alternate fire";
		case AlternateFireModeOn: return "15 alternate fire mode on";
		case DryFire: return "16 dryfire";
		case Reload: return "17 reload";
		case EmptyReload: return "18 empty reload";
	}
}

class Character {
private:
	AnimationInstance animationInstance;
	State state;

	int bulletsInClip;
	int totalAmmo;
	int health;

	float movementSpeed = 5.f;
	float sprintSpeed = 10.f;
	float positionX;
	float positionY;
public:
	AnimatedModel animatedModel;

	void initialize(Core* core, PSOManager* psos, TextureManager* textures, ShaderManager* shaders, std::string filename) {
		animatedModel.load(core, psos, textures, shaders, filename);
		animationInstance.initialize(&animatedModel.animation, 0);
		state = Pose;
	}

	void setAnmationState(State _state) { state = _state; }
	void updateAnimation(float dt) { animationInstance.update(getAnimationByState(state), dt); }
	void resetAnimationTime() { if (animationInstance.animationFinished() == true) animationInstance.resetAnimationTime(); }

	void draw(Core* core, TextureManager* textures, PSOManager* psos, ShaderManager* shaders, Matrix& vp, Matrix& w) {
		animatedModel.draw(core, &animationInstance, textures, psos, shaders, vp, w);
	}

	void movePlayer(Camera* camera, Window* window, float dt) {
		float speed = (window->keys[VK_SHIFT] == 1) ? sprintSpeed : movementSpeed;
		State animState = (window->keys[VK_SHIFT] == 1) ? Run : Walk;

		// Camera and Player Movement via Keyboard
		if (window->keys['W'] == 1) {
			camera->walk(speed * dt);
			positionY += speed * dt;
		}

		if (window->keys['S'] == 1) {
			camera->walk(-speed * dt);
			positionY -= speed * dt;
		}

		if (window->keys['A'] == 1) {
			camera->strafe(-speed * dt);
			positionX -= speed * dt;
		}

		if (window->keys['D'] == 1) {
			camera->strafe(speed * dt);
			positionX += speed * dt;
		}

		updateAnimation(dt);
		resetAnimationTime();
		camera->updateViewMatrix();
	}

	// Make functions for idle and shoot (and their zoomed ver. i.e. hold right click)...
};