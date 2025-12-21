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

	std::string shadername;
	std::string psoname;
	
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
		shadername = "StaticModelTextured";
		psoname = "StaticModelPSO";
		shaders->loadShader(core, shadername, "VertexShaderStatic.txt", "PixelShaderMultipleTextured.txt");
		psos->createPSO(core, psoname, shaders->getShader(shadername)->vertexShader, shaders->getShader(shadername)->pixelShader, VertexLayoutCache::getStaticLayout());
	}

	void draw(Core* core, PSOManager* psos, TextureManager* textures, ShaderManager* shaders, Matrix& vp, Matrix& w) {
		psos->bind(core, psoname);
		shaders->updateConstantVertexShaderBuffer(shadername, "staticMeshBuffer", "W", &w);
		shaders->updateConstantVertexShaderBuffer(shadername, "staticMeshBuffer", "VP", &vp);
		shaders->apply(core, shadername);

		for (int i = 0; i < meshes.size(); i++) {
			std::cout << albedoFilenames[i] << ' ' << textures->find(albedoFilenames[i]) << '\n';
			std::cout << normalFilenames[i] << ' ' << textures->find(normalFilenames[i]) << '\n';
			std::cout << roughnessFilenames[i] << ' ' << textures->find(roughnessFilenames[i]) << '\n';

			shaders->updateTexturePS(core, shadername, "albedoTexture", textures->find(albedoFilenames[i]));
			shaders->updateTexturePS(core, shadername, "normalsTexture", textures->find(normalFilenames[i]));
			shaders->updateTexturePS(core, shadername, "roughnessTexture", textures->find(roughnessFilenames[i]));
			
			meshes[i]->draw(core);
		}
	}
};

class StaticInstancedModel {
public:
	std::vector<Mesh*> meshes;
	std::vector<INSTANCE_DATA> instances;

	std::vector<std::string> albedoFilenames;
	std::vector<std::string> normalFilenames;
	std::vector<std::string> roughnessFilenames;

	std::string shadername;
	std::string psoname;

	void load(Core* core, PSOManager* psos, TextureManager* textures, ShaderManager* shaders, std::string filename, std::vector<INSTANCE_DATA>& worlds) {
		GEMLoader::GEMModelLoader loader;
		std::vector<GEMLoader::GEMMesh> gemmeshes;
		loader.load(filename, gemmeshes);
		instances = worlds;

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
			
			mesh->initialize(core, vertices, gemmeshes[i].indices, instances);
			meshes.push_back(mesh);
		}
		shadername = "StaticModelTexturedInstanced";
		psoname = "StaticModelInstancedPSO";
		shaders->loadShader(core, shadername, "VertexShaderStaticInstanced.txt", "PixelShaderMultipleTextured.txt");
		psos->createPSO(core, psoname, shaders->getShader(shadername)->vertexShader, shaders->getShader(shadername)->pixelShader, VertexLayoutCache::getStaticInstancedLayout());
	}

	void draw(Core* core, PSOManager* psos, TextureManager* textures, ShaderManager* shaders, Matrix& vp) {
		psos->bind(core, psoname);
		shaders->updateConstantVertexShaderBuffer(shadername, "staticInstacedMeshBuffer", "VP", &vp);
		shaders->apply(core, shadername);

		for (int i = 0; i < meshes.size(); i++) {
			std::cout << albedoFilenames[i] << ' ' << textures->find(albedoFilenames[i]) << '\n';
			std::cout << normalFilenames[i] << ' ' << textures->find(normalFilenames[i]) << '\n';
			std::cout << roughnessFilenames[i] << ' ' << textures->find(roughnessFilenames[i]) << '\n';

			shaders->updateTexturePS(core, shadername, "albedoTexture", textures->find(albedoFilenames[i]));
			shaders->updateTexturePS(core, shadername, "normalsTexture", textures->find(normalFilenames[i]));
			shaders->updateTexturePS(core, shadername, "roughnessTexture", textures->find(roughnessFilenames[i]));

			meshes[i]->draw(core);
		}
	}
};

class StaticInstancedVertexAnimModel {
public:
	std::vector<Mesh*> meshes;
	std::vector<INSTANCE_DATA> instances;

	std::vector<std::string> albedoFilenames;
	std::vector<std::string> normalFilenames;

	std::string shadername;
	std::string psoname;

	void load(Core* core, PSOManager* psos, TextureManager* textures, ShaderManager* shaders, std::string filename, std::vector<INSTANCE_DATA>& worlds) {
		GEMLoader::GEMModelLoader loader;
		std::vector<GEMLoader::GEMMesh> gemmeshes;
		loader.load(filename, gemmeshes);
		instances = worlds;

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

			textures->loadTexture(core, albedoFilenames[i], gemmeshes[i].material.find("albedo").getValue());
			textures->loadTexture(core, normalFilenames[i], gemmeshes[i].material.find("nh").getValue());

			mesh->initialize(core, vertices, gemmeshes[i].indices, instances);
			meshes.push_back(mesh);
		}
		shadername = "StaticModelTexturedVertexAnimInstanced";
		psoname = "StaticModelInstancedVertexAnimPSO";
		shaders->loadShader(core, shadername, "VertexShaderInstancedGrass.txt", "PixelShaderGrass.txt");
		psos->createPSO(core, psoname, shaders->getShader(shadername)->vertexShader, shaders->getShader(shadername)->pixelShader, VertexLayoutCache::getStaticInstancedLayout());
	}

	void draw(Core* core, PSOManager* psos, TextureManager* textures, ShaderManager* shaders, Matrix& vp, float time) {
		psos->bind(core, psoname);
		shaders->updateConstantVertexShaderBuffer(shadername, "staticInstacedMeshBuffer", "VP", &vp);
		shaders->updateConstantVertexShaderBuffer(shadername, "staticInstacedMeshBuffer", "Time", &time);
		shaders->apply(core, shadername);

		for (int i = 0; i < meshes.size(); i++) {
			std::cout << albedoFilenames[i] << ' ' << textures->find(albedoFilenames[i]) << '\n';
			std::cout << normalFilenames[i] << ' ' << textures->find(normalFilenames[i]) << '\n';

			shaders->updateTexturePS(core, shadername, "albedoTexture", textures->find(albedoFilenames[i]));
			shaders->updateTexturePS(core, shadername, "normalsTexture", textures->find(normalFilenames[i]));

			meshes[i]->draw(core);
		}
	}
};