#pragma once

#include <vector>

#include "Core.h"
#include "Mesh.h"
#include "PSOManager.h"
#include "Shaders.h"

class Plane {
public:
	Mesh mesh;
	std::string shadername;

	STATIC_VERTEX addVertex(Vec3 p, Vec3 n, float tu, float tv) {
		STATIC_VERTEX v;
		v.pos = p;
		v.normal = n;
		v.tangent = Vec3(0.f, 0.f, 0.f); // For now
		v.tu = tu;
		v.tv = tv;
		return v;
	}

	void initialize(Core* core, PSOManager* psos, ShaderManager* shaders) {
		std::vector<STATIC_VERTEX> vertices;
		vertices.push_back(addVertex(Vec3(-1.f, 0.f, -1.f), Vec3(0.f, 1.f, 0.f), 0.f, 0.f));
		vertices.push_back(addVertex(Vec3(1.f, 0.f, -1.f), Vec3(0.f, 1.f, 0.f), 1.f, 0.f));
		vertices.push_back(addVertex(Vec3(-1.f, 0.f, 1.f), Vec3(0.f, 1.f, 0.f), 0.f, 1.f));
		vertices.push_back(addVertex(Vec3(1.f, 0.f, 1.f), Vec3(0.f, 1.f, 0.f), 1.f, 1.f));

		std::vector<unsigned int> indices;
		indices.push_back(2); indices.push_back(1); indices.push_back(0);
		indices.push_back(1); indices.push_back(2); indices.push_back(3);

		mesh.initialize(core, vertices, indices);
		shaders->loadShader(
			core, "Plane",
			"C:/dev/WM9M2Assignment5749205/WM9M2Assignment5749205/VertexShader.txt", 
			"C:/dev/WM9M2Assignment5749205/WM9M2Assignment5749205/PixelShader.txt"
		);
		shadername = "Plane";
		psos->createPSO(core, "PlanePSO", shaders->getShader(shadername)->vertexShader, shaders->getShader(shadername)->pixelShader, VertexLayoutCache::getStaticLayout());
	}

	void draw(Core* core, PSOManager* psos, ShaderManager* shaders) {
		Matrix planeWorld = Matrix::identity();
		shaders->updateConstantVertexShaderBuffer("Plane", "staticMeshBuffer", "W", &planeWorld);
		shaders->apply(core, shadername);
		psos->bind(core, "PlanePSO");
		mesh.draw(core);
	}
};