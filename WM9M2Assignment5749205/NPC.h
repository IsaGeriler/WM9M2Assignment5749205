#pragma once

#include "AnimatedModel.h"
#include "Core.h"

enum NPCState {
	NPCAttack,
	NPCDeath,
	NPCIdle,
	NPCIdle2,
	NPCRoar,
	NPCRun,
	NPCWalk
};

std::string getNPCAnimationByState(NPCState npcstate) {
	switch (npcstate) {
		case NPCAttack: { return "attack"; break; }
		case NPCDeath: { return "death"; break; }
		case NPCIdle: { return "idle"; break; }
		case NPCIdle2: { return "idle2"; break; }
		case NPCRoar: { return "roar"; break; }
		case NPCRun: { return "run"; break; }
		case NPCWalk: { return "walk"; break; }
		default: { return "idle";  break; }
	}
}

class NPC {
private:
	AnimationInstance animationInstance;
	NPCState npcstate;

	int health{ 5000 };

	bool isAlive{ true };
	
	float movementSpeed = 2.5f;
	float sprintSpeed = 25.f;
	float positionX{ 0.f };
	float positionY{ 0.f };

	void setAnimationState(NPCState _npcstate) { npcstate = _npcstate; }
	void updateAnimation(float dt) { animationInstance.update(getNPCAnimationByState(npcstate), dt); }
	void resetAnimationTime() { if (animationInstance.animationFinished() == true) animationInstance.resetAnimationTime(); }
public:
	AnimatedModel animatedModel;

	void initialize(Core* core, PSOManager* psos, TextureManager* textures, ShaderManager* shaders, std::string filename) {
		animatedModel.load(core, psos, textures, shaders, filename);  // "Models/TRex.gem"
		animationInstance.initialize(&animatedModel.animation, 0);
		npcstate = NPCWalk;
	}

	void takeDamage(int damage) {
		if (!isAlive) return;
		health -= damage;
		if (health <= 0) {
			isAlive = false;
			npcstate = NPCDeath;
		}
	}

	void animate(float dt) {
		updateAnimation(dt);
		resetAnimationTime();
	}

	void draw(Core* core, TextureManager* textures, PSOManager* psos, ShaderManager* shaders, Matrix& vp, Matrix& w) {
		if (!isAlive && animationInstance.animationFinished()) return;
		animatedModel.draw(core, &animationInstance, textures, psos, shaders, vp, w);
	}
};