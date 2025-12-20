#pragma once

#include "AnimatedModel.h"
#include "Core.h"

enum NPCState {
	Attack,
	Death,
	Idle,
	Idle2,
	Roar,
	Run,
	Walk
};

std::string getNPCAnimationByState(NPCState npcstate) {
	switch (npcstate) {
		case Attack: { return "attack"; break; }
		case Death: { return "death"; break; }
		case Idle: { return "idle"; break; }
		case Idle2: { return "idle2"; break; }
		case Roar: { return "roar"; break; }
		case Run: { return "run"; break; }
		case Walk: { return "walk"; break; }
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
	float sprintSpeed = 20.f;
	float positionX{ 0.f };
	float positionY{ 0.f };
public:
	AnimatedModel animatedModel;

	void initialize(Core* core, PSOManager* psos, TextureManager* textures, ShaderManager* shaders, std::string filename) {
		animatedModel.load(core, psos, textures, shaders, filename);  // "Models/TRex.gem"
		animationInstance.initialize(&animatedModel.animation, 0);
		npcstate = Idle;
	}

	void takeDamage(int damage) {
		if (!isAlive) return;
		health -= damage;
		if (health <= 0) {
			isAlive = false;
			npcstate = Death;
		}
	}

	void draw(Core* core, TextureManager* textures, PSOManager* psos, ShaderManager* shaders, Matrix& vp, Matrix& w) {
		animatedModel.draw(core, &animationInstance, textures, psos, shaders, vp, w);
	}
};