#pragma once

#include <vector>

#include "Core.h"
#include "GEMLoader.h"
#include "Mesh.h"
#include "PSOManager.h"
#include "Shaders.h"

class StaticModel {
public:
	std::vector<Mesh*> meshes;
	std::string shadername;

	void load(Core* core, PSOManager* psos, ShaderManager* shaders, std::string filename) {
		GEMLoader::GEMModelLoader loader;
		std::vector<GEMLoader::GEMMesh> gemmeshes;
		loader.load(filename, gemmeshes);
		for (int i = 0; i < gemmeshes.size(); i++) {
			Mesh* mesh = new Mesh();
			std::vector<STATIC_VERTEX> vertices;
			for (int j = 0; j < gemmeshes[i].verticesStatic.size(); j++) {
				STATIC_VERTEX v;
				memcpy(&v, &gemmeshes[i].verticesStatic[j], sizeof(STATIC_VERTEX));
				vertices.push_back(v);
			}
			mesh->initialize(core, vertices, gemmeshes[i].indices);
			meshes.push_back(mesh);
		}
		shaders->loadShader(core, "StaticModelUntextured", "VertexShader.txt", "PixelShader.txt");
		shadername = "StaticModelUntextured";
		psos->createPSO(core, "StaticModelPSO", shaders->getShader("StaticModelUntextured")->vertexShader, shaders->getShader("StaticModelUntextured")->pixelShader, VertexLayoutCache::getStaticLayout());
	}

	void draw(Core* core, PSOManager* psos, ShaderManager* shaders) {
		shaders->apply(core, shadername);
		psos->bind(core, "SpherePSO");
		for (int i = 0; i < meshes.size(); i++) meshes[i]->draw(core);
	}
};