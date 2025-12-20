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
public:
	AnimatedModel animatedModel;

	void initialize() {
	}
};