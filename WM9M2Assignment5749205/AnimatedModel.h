#pragma once

#include "Animation.h"
#include "Core.h"
#include "GEMLoader.h"
#include "Mesh.h"
#include "PSOManager.h"
#include "Shaders.h"

class AnimatedModel {
public:
	std::vector<Mesh*> meshes;
	Animation animation;
	std::string shadername;

	void load(Core* core, PSOManager* psos, ShaderManager* shaders, std::string filename) {
		// Load in animation data
		GEMLoader::GEMModelLoader loader;
		std::vector<GEMLoader::GEMMesh> gemmeshes;
		GEMLoader::GEMAnimation gemanimation;
		loader.load(filename, gemmeshes, gemanimation);
		for (int i = 0; i < gemmeshes.size(); i++) {
			Mesh* mesh = new Mesh();
			std::vector<ANIMATED_VERTEX> vertices;
			for (int j = 0; j < gemmeshes[i].verticesAnimated.size(); j++) {
				ANIMATED_VERTEX v;
				memcpy(&v, &gemmeshes[i].verticesAnimated[j], sizeof(ANIMATED_VERTEX));
				vertices.push_back(v);
			}
			mesh->initialize(core, vertices, gemmeshes[i].indices);
			meshes.push_back(mesh);
		}
		shaders->loadShader(core, "AnimatedModelUntextured", "VertexShaderAnimated.txt", "PixelShader.txt");
		shadername = "AnimatedModelUntextured";
		psos->createPSO(core, "AnimatedModelPSO", shaders->getShader("AnimatedModelUntextured")->vertexShader, shaders->getShader("AnimatedModelUntextured")->pixelShader, VertexLayoutCache::getAnimatedLayout());
		memcpy(&animation.skeleton.globalInverse, &gemanimation.globalInverse, 16 * sizeof(float));

		// Bones
		for (int i = 0; i < gemanimation.bones.size(); i++) {
			Bone bone;
			bone.name = gemanimation.bones[i].name;
			memcpy(&bone.offset, &gemanimation.bones[i].offset, 16 * sizeof(float));
			bone.parentIndex = gemanimation.bones[i].parentIndex;
			animation.skeleton.bones.push_back(bone);
		}

		// Animations (Copy Data)
		for (int i = 0; i < gemanimation.animations.size(); i++) {
			std::string name = gemanimation.animations[i].name;
			AnimationSequence aseq;
			aseq.ticksPerSecond = gemanimation.animations[i].ticksPerSecond;
			for (int n = 0; n < gemanimation.animations[i].frames.size(); n++) {
				AnimationFrame frame;
				for (int index = 0; index < gemanimation.animations[i].frames[n].positions.size(); index++) {
					Vec3 p;
					Quaternion q;
					Vec3 s;
					memcpy(&p, &gemanimation.animations[i].frames[n].positions[index], sizeof(Vec3));
					frame.positions.push_back(p);
					memcpy(&q, &gemanimation.animations[i].frames[n].rotations[index], sizeof(Quaternion));
					frame.rotations.push_back(q);
					memcpy(&s, &gemanimation.animations[i].frames[n].scales[index], sizeof(Vec3));
					frame.scales.push_back(s);
				}
				aseq.frames.push_back(frame);
			}
			animation.animations.insert({ name, aseq });
		}
	}

	void draw(Core* core, PSOManager* psos, ShaderManager* shaders, AnimationInstance* instance, Matrix& vp, Matrix& w) {
		psos->bind(core, "AnimatedModelPSO");
		shaders->updateConstantVertexShaderBuffer("AnimatedModelUntextured", "animatedMeshBuffer", "W", &w);
		shaders->updateConstantVertexShaderBuffer("AnimatedModelUntextured", "animatedMeshBuffer", "VP", &vp);
		shaders->updateConstantVertexShaderBuffer("AnimatedModelUntextured", "animatedMeshBuffer", "bones", instance->matrices);
		shaders->apply(core, "AnimatedModelUntextured");
		for (int i = 0; i < meshes.size(); i++) meshes[i]->draw(core);
	}
};