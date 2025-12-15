#pragma once

#include <vector>

#include "Core.h"
#include "GEMLoader.h"
#include "Mesh.h"
#include "PSOManager.h"
#include "Shaders.h"
#include "Texture.h"

class StaticModel {
public:
	std::vector<Mesh*> meshes;
	std::vector<std::string> albedoFilenames;
	std::vector<std::string> normalFilenames;
	std::vector<std::string> roughnessFilenames;
	
	void load(Core* core, PSOManager* psos, TextureManager* textures, ShaderManager* shaders, std::string filename) {
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
			// Load texture with filename: gemmeshes[i].material.find("albedo").getValue()
			albedoFilenames.push_back(gemmeshes[i].material.find("albedo").getValue());
			normalFilenames.push_back(gemmeshes[i].material.find("nh").getValue());
			roughnessFilenames.push_back(gemmeshes[i].material.find("rmax").getValue());
			
			textures->loadTexture(core, albedoFilenames[i], gemmeshes[i].material.find("albedo").getValue());
			textures->loadTexture(core, normalFilenames[i], gemmeshes[i].material.find("nh").getValue());
			textures->loadTexture(core, roughnessFilenames[i], gemmeshes[i].material.find("rmax").getValue());
			
			mesh->initialize(core, vertices, gemmeshes[i].indices);
			meshes.push_back(mesh);
		}
		shaders->loadShader(core, "StaticModelTextured", "VertexShaderStatic.txt", "PixelShaderMultipleTextured.txt");
		psos->createPSO(core, "StaticModelPSO", shaders->getShader("StaticModelTextured")->vertexShader, shaders->getShader("StaticModelTextured")->pixelShader, VertexLayoutCache::getStaticLayout());
	}

	void draw(Core* core, PSOManager* psos, TextureManager* textures, ShaderManager* shaders, Matrix& vp, Matrix& w) {
		psos->bind(core, "StaticModelPSO");
		shaders->updateConstantVertexShaderBuffer("StaticModelTextured", "staticMeshBuffer", "W", &w);
		shaders->updateConstantVertexShaderBuffer("StaticModelTextured", "staticMeshBuffer", "VP", &vp);
		shaders->apply(core, "StaticModelTextured");

		for (int i = 0; i < meshes.size(); i++) {
			std::cout << albedoFilenames[i] << ' ' << textures->find(albedoFilenames[i]) << '\n';
			std::cout << normalFilenames[i] << ' ' << textures->find(normalFilenames[i]) << '\n';
			std::cout << roughnessFilenames[i] << ' ' << textures->find(roughnessFilenames[i]) << '\n';

			// Normal map can be found in gemmeshes[i].material.find("nh").getValue()
			// The rgb channels contain the normal map, and the alpha channel contains a height map

			shaders->updateTexturePS(core, "StaticModelTextured", "albedoTexture", textures->find(albedoFilenames[i]));
			shaders->updateTexturePS(core, "StaticModelTextured", "normalsTexture", textures->find(normalFilenames[i]));
			shaders->updateTexturePS(core, "StaticModelTextured", "roughnessTexture", textures->find(roughnessFilenames[i]));
			
			meshes[i]->draw(core);
		}
	}
};