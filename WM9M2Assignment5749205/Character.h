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
		case Pose: { return "00 pose"; break; }
		case Select: { return "01 select"; break; }
		case Putaway: { return "02 putaway"; break; }
		case EmptySelect: { return "03 empty select"; break; }
		case Idle: { return "04 idle"; break; }
		case Inspect: { return "05 inspect"; break; }
		case Walk: { return "06 walk"; break; }
		case Run: { return "07 run"; break; }
		case Fire: { return "08 fire"; break; }
		case AlternateFire: { return "09 alternate fire"; break; }
		case MeleeAttack: { return "10 melee attack"; break; }
		case ZoomIdle: { return "11 zoom idle"; break; }
		case ZoomWalk: { return "12 zoom walk"; break; }
		case ZoomFire: { return "13 zoom fire"; break; }
		case ZoomAlternateFire: { return "14 zoom alternate fire"; break; }
		case AlternateFireModeOn: { return "15 alternate fire mode on"; break; }
		case DryFire: { return "16 dryfire"; break; }
		case Reload: { return "17 reload"; break; }
		case EmptyReload: { return "18 empty reload"; break; }
		default: { return "04 idle"; break; }
	}
}

class Character {
private:
	AnimationInstance animationInstance;
	State state;

	int bulletsInClip{ 0 };
	int totalAmmo{ 240 };
	int health{ 100 };

	float movementSpeed = 5.f;
	float sprintSpeed = 10.f;
	float positionX{ 0.f };
	float positionY{ 0.f };

	bool carryGun = true;
	bool toggleAlternateFire = false;
	bool zoom = false;

	int animCounter{ 0 };
public:
	AnimatedModel animatedModel;

	void initialize(Core* core, PSOManager* psos, TextureManager* textures, ShaderManager* shaders, std::string filename) {
		animatedModel.load(core, psos, textures, shaders, filename);
		animationInstance.initialize(&animatedModel.animation, 0);
		state = Pose;
	}

	void setAnimationState(State _state) { state = _state; }
	void updateAnimation(float dt) { animationInstance.update(getAnimationByState(state), dt); }
	void resetAnimationTime() { if (animationInstance.animationFinished() == true) animationInstance.resetAnimationTime(); }

	void draw(Core* core, TextureManager* textures, PSOManager* psos, ShaderManager* shaders, Matrix& vp, Matrix& w) {
		animatedModel.draw(core, &animationInstance, textures, psos, shaders, vp, w);
	}

	void movePlayer(Camera* camera, Window* window, float dt) {
		float speed = (window->keys[VK_SHIFT] == 1) ? sprintSpeed : movementSpeed;
		State state;
		if (zoom) state = ZoomWalk;
		else state = (window->keys[VK_SHIFT] == 1) ? Run : Walk;

		// Camera and Player Movement via Keyboard
		if (window->keys['W'] == 1 && carryGun) {
			setAnimationState(state);
			camera->walk(speed * dt);
			positionY += speed * dt;
		}

		if (window->keys['S'] == 1 && carryGun) {
			setAnimationState(state);
			camera->walk(-speed * dt);
			positionY -= speed * dt;
		}

		if (window->keys['A'] == 1) {
			setAnimationState(state);
			camera->strafe(-speed * dt);
			positionX -= speed * dt;
		}

		if (window->keys['D'] == 1) {
			setAnimationState(state);
			camera->strafe(speed * dt);
			positionX += speed * dt;
		}
		camera->updateViewMatrix();
		animate(dt, true);
	}

	// Inspect Weapon
	void inspectWeapon(Window* window, float dt) {
		if (window->keys['I'] == 1 && carryGun) setAnimationState(Inspect);
		if (animationInstance.animationFinished()) setAnimationState(Idle);
		animate(dt, false);
	}

	// Perform Melee Attack (needs collision detection for damage)
	void meleeAttack(Window* window, float dt) {
		if (window->keys['M'] == 1 && carryGun) setAnimationState(MeleeAttack);
		if (animationInstance.animationFinished()) setAnimationState(Idle);
		animate(dt, false);
	}

	// Putaway the carbine
	void putawayWeapon(Window* window, float dt) {
		if (window->keys['P'] && carryGun == true) {
			carryGun = false;
			setAnimationState(Putaway);
		}
		if (animationInstance.animationFinished()) setAnimationState(Idle);
		animate(dt, false);
	}

	// Select the carbine
	void selectWeapon(Window* window, float dt) {
		if (window->keys['O'] && carryGun == false) {
			carryGun = true;
			if (bulletsInClip == 0) setAnimationState(EmptySelect);
			else setAnimationState(Select);
		}
		if (animationInstance.animationFinished()) setAnimationState(Idle);
		animate(dt, false);
	}

	// Reload
	void reload(Window* window, float dt) {
		if (window->keys['R'] && (bulletsInClip < 30 && totalAmmo > 0)) {
			State state = (bulletsInClip == 0) ? EmptyReload : Reload;
			bulletsInClip = std::min<int>(30, totalAmmo);
			totalAmmo -= bulletsInClip;
			setAnimationState(state);
		}
		if (animationInstance.animationFinished()) setAnimationState(Idle);
		animate(dt, false);
	}

	// Toggle Alternate Fire Mode
	void toggleAlternateFireMode(Window* window, float dt) {
		if (window->keys[VK_SPACE]) {
			toggleAlternateFire = !toggleAlternateFire;
			setAnimationState(AlternateFireModeOn);
		}
		if (animationInstance.animationFinished()) setAnimationState(Idle);
		animate(dt, false);
	}

	// Shoot Bullet (needs collision detection for damage)
	void shoot(Window* window, float dt) {
		if (window->keys['F'] == 1) {
			State state;
			if (bulletsInClip > 0) {
				int deduceBullet = (toggleAlternateFire) ? 3 : 1;
				if (zoom) state = ZoomIdle;
				else state = (toggleAlternateFire) ? Fire : AlternateFire;
				bulletsInClip -= std::max<int>(deduceBullet, bulletsInClip);
				setAnimationState(state);
			} else {
				setAnimationState(DryFire);
			}
		}
		if (animationInstance.animationFinished()) setAnimationState(Idle);
		animate(dt, false);
	}

	void animate(float dt, bool continuous) {
		updateAnimation(dt);
		if (continuous && animationInstance.animationFinished()) resetAnimationTime();
	}

	void resetAnimationState() {
		if (zoom) state = ZoomIdle;
		else state = Idle;
	}
};