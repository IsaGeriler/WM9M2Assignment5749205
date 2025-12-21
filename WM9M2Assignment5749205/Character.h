#pragma once

#include <string>

#include "AnimatedModel.h"
#include "Camera.h"
#include "Collision.h"
#include "Core.h"
#include "MyMath.h"
#include "Window.h"

// Animated Carbine
enum State {
	Pose, Select, Putaway, EmptySelect, Idle,
	Inspect, Walk, Run, Fire, AlternateFire,
	MeleeAttack, AlternateFireModeOn,
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
	BoundingSphere hitbox;
	State state;

	int bulletsInClip{ 30 };
	int totalAmmo{ 240 };
	int health{ 100 };
	int bulletDamage{ 50 };

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
		hitbox.centre = Vec3(0.f, 0.f, 0.f);
		hitbox.radius = 1.f;
	}

	void setAnimationState(State _state) { state = _state; }
	void updateAnimation(float dt) { animationInstance.update(getAnimationByState(state), dt); }
	void resetAnimationTime() { if (animationInstance.animationFinished() == true) animationInstance.resetAnimationTime(); }

	void draw(Core* core, TextureManager* textures, PSOManager* psos, ShaderManager* shaders, Matrix& vp, Matrix& w) {
		animatedModel.draw(core, &animationInstance, textures, psos, shaders, vp, w);
	}

	void movePlayer(Camera* camera, Window* window, float dt) {
		float speed = (window->keys[VK_SHIFT] == 1) ? sprintSpeed : movementSpeed;
		State state = (window->keys[VK_SHIFT] == 1) ? Run : Walk;

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
		hitbox.centre = camera->position;
	}

	// Inspect Weapon
	void inspectWeapon() {
		if (animationInstance.animationFinished()) { setAnimationState(Idle); return; }
		if (carryGun) setAnimationState(Inspect);
	}

	// Perform Melee Attack (needs collision detection for damage)
	void meleeAttack() {
		if (carryGun) setAnimationState(MeleeAttack);
		if (animationInstance.animationFinished()) { setAnimationState(Idle); return; }
	}

	// Putaway the carbine
	void putawayWeapon() {
		if (carryGun) {
			carryGun = !carryGun;
			setAnimationState(Putaway);
		}
		if (animationInstance.animationFinished()) { setAnimationState(Idle); return; }
	}

	// Select the carbine
	void selectWeapon() {
		if (!carryGun) {
			carryGun = !carryGun;
			State state = (bulletsInClip == 0) ? EmptySelect : Select;
			setAnimationState(state);
		}
		if (animationInstance.animationFinished()) { setAnimationState(Idle); return; }
	}

	// Reload
	void reload() {
		if (bulletsInClip < 30 && totalAmmo > 0) {
			State state = (bulletsInClip == 0) ? EmptyReload : Reload;
			bulletsInClip = std::min<int>(30, totalAmmo);
			totalAmmo -= bulletsInClip;
			setAnimationState(state);
		}
		if (animationInstance.animationFinished()) { setAnimationState(Idle); return; }
	}

	// Toggle Alternate Fire Mode
	void toggleAlternateFireMode() {
		toggleAlternateFire = !toggleAlternateFire;
		setAnimationState(AlternateFireModeOn);
		if (animationInstance.animationFinished()) { setAnimationState(Idle); return; }
	}

	// Shoot Bullet (needs collision detection for damage)
	void shoot() {
		State state;
		if (bulletsInClip > 0) {
			int deduceBullet = (toggleAlternateFire) ? 3 : 1;
			state = (toggleAlternateFire) ? Fire : AlternateFire;
			bulletsInClip -= std::max<int>(deduceBullet, bulletsInClip);
			setAnimationState(state);
		} else {
			setAnimationState(DryFire);
		}
		if (animationInstance.animationFinished()) { setAnimationState(Idle); return; }
	}

	void animate(float dt) {
		updateAnimation(dt);
		resetAnimationTime();
	}

	void resetAnimationState() {
		state = Idle;
	}
};