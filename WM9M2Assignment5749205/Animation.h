#pragma once

#include <string>
#include <map>
#include <vector>

#include "MyMath.h"

struct Bone {
	std::string name;
	Matrix offset;
	int parentIndex;
};

struct Skeleton {
	std::vector<Bone> bones;
	Matrix globalInverse;

	int findBone(std::string name) {
		for (int i = 0; i < bones.size(); i++)
			if (bones[i].name == name) return i;
		return -1;
	}
};

struct AnimationFrame {
	std::vector<Vec3> positions;
	std::vector<Quaternion> rotations;
	std::vector<Vec3> scales;
};

struct AnimationSequence {
	std::vector<AnimationFrame> frames;
	float ticksPerSecond;
	
	Vec3 interpolate(Vec3 p1, Vec3 p2, float t) {
		return ((p1 * (1.0f - t)) + (p2 * t));
	}
	
	Quaternion interpolate(Quaternion q1, Quaternion q2, float t) {
		return slerp(q1, q2, t);
	}
	
	float duration() {
		return ((float)frames.size() / ticksPerSecond);
	}

	void calcFrame(float t, int& frame, float& interpolationFact) {
		interpolationFact = t * ticksPerSecond;
		frame = (int)floorf(interpolationFact);
		interpolationFact = interpolationFact - (float)frame;
		frame = std::min<int>(frame, (int)(frames.size() - 1));
	}

	bool running(float t) {
		return ((int)floorf(t * ticksPerSecond) < frames.size());
	}
	
	int nextFrame(int frame) {
		return std::min<int>(frame + 1, (int)(frames.size() - 1));
	}

	Matrix interpolateBoneToGlobal(Matrix* matrices, int baseFrame, float interpolationFact, Skeleton* skeleton, int boneIndex) {
		Matrix scale = Matrix::scale(interpolate(frames[baseFrame].scales[boneIndex], frames[nextFrame(baseFrame)].scales[boneIndex], interpolationFact));
		Matrix rotation = interpolate(frames[baseFrame].rotations[boneIndex], frames[nextFrame(baseFrame)].rotations[boneIndex], interpolationFact).toMatrix();
		Matrix translation = Matrix::translate(interpolate(frames[baseFrame].positions[boneIndex], frames[nextFrame(baseFrame)].positions[boneIndex], interpolationFact));
		Matrix local =  scale * rotation * translation;
		
		if (skeleton->bones[boneIndex].parentIndex > -1) {
			Matrix global = local * matrices[skeleton->bones[boneIndex].parentIndex];
			return global;
		}
		return local;
	}
};

class Animation {
public:
	std::map<std::string, AnimationSequence> animations;
	Skeleton skeleton;

	void calcFrame(std::string name, float t, int& frame, float& interpolationFact) {
		animations[name].calcFrame(t, frame, interpolationFact);
	}
	
	Matrix interpolateBoneToGlobal(std::string name, Matrix* matrices, int baseFrame, float interpolationFact, int boneIndex) {
		return animations[name].interpolateBoneToGlobal(matrices, baseFrame, interpolationFact, &skeleton, boneIndex);
	}

	void calcFinalTransforms(Matrix* matrices, Matrix coordTransform) {
		for (int i = 0; i < skeleton.bones.size(); i++)
			matrices[i] = skeleton.bones[i].offset * matrices[i] * skeleton.globalInverse * coordTransform;
	}

	bool hasAnimation(std::string name) {
		return !(animations.find(name) == animations.end());
	}
};

class AnimationInstance {
public:
	Animation* animation;
	std::string currentAnimation;
	float t;
	Matrix matrices[256];
	Matrix matricesPose[256];
	Matrix coordTransform;

	void initialize(Animation* _animation, int fromYZX) {
		animation = _animation;
		if (fromYZX == 1) {
			memset(coordTransform.a, 0, 16 * sizeof(float));
			coordTransform.a[0][0] = 1.0f;
			coordTransform.a[2][1] = 1.0f;
			coordTransform.a[1][2] = -1.0f;
			coordTransform.a[3][3] = 1.0f;
		}
	}

	void resetAnimationTime() {
		t = 0;
	}
	
	bool animationFinished() {
		return (t > animation->animations[currentAnimation].duration());
	}

	void update(std::string name, float dt) {
		if (name == currentAnimation) t += dt;
		else {
			currentAnimation = name;
			t = 0;
		}

		if (animationFinished() == true) return;

		int frame = 0;
		float interpolationFact = 0;
		animation->calcFrame(name, t, frame, interpolationFact);
		for (int i = 0; i < animation->skeleton.bones.size(); i++)
			matrices[i] = animation->interpolateBoneToGlobal(name, matrices, frame, interpolationFact, i);
		animation->calcFinalTransforms(matrices, coordTransform);
	}

	Matrix findWorldMatrix(std::string bonename) {
		int boneID = animation->skeleton.findBone(bonename);
		std::vector<int> boneChain;
		int id = boneID;

		while (id != 1) {
			boneChain.push_back(id);
			id = animation->skeleton.bones[id].parentIndex;
		}

		int frame = 0;
		float interpolationFact = 0;
		animation->calcFrame(currentAnimation, t, frame, interpolationFact);
		
		for (int i = boneChain.size() - 1; i > -1; i = i - 1)
			matricesPose[boneChain[i]] = animation->interpolateBoneToGlobal(currentAnimation, matricesPose, frame, interpolationFact, boneChain[i]);
		return (matricesPose[boneID] * coordTransform);
	}
};