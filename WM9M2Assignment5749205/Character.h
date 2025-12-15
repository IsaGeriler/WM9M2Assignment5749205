#pragma once

#include <string>

#include "AnimatedModel.h"
#include "Core.h"
#include "MyMath.h"

// Animated Carbine
enum State {
	Pose,
	Select,
	Putaway,
	EmptySelect,
	Idle,
	Inspect,
	Walk,
	Run,
	Fire,
	AlternateFire,
	MeleeAttack,
	ZoomIdle,
	ZoomWalk,
	ZoomFire,
	ZoomAlternateFire,
	AlternateFireModeOn,
	DryFire,
	Reload,
	EmptyReload
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

	// Bullet in clip
	// Total magazine/ammo count
	// Health
public:
	AnimatedModel animatedModel;

	void initialize(Core* core, PSOManager* psos, TextureManager* textures, ShaderManager* shaders, std::string filename) {
		animatedModel.load(core, psos, textures, shaders, filename);
		animationInstance.animation = &animatedModel.animation;
		state = Pose;
	}

	// Make function for idle, walk, sprint, shoot (and their zoomed ver. i.e. hold right click)
};