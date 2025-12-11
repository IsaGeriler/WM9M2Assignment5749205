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

	std::vector<STATIC_VERTEX> vertices;
	std::vector<unsigned int> indices;

	STATIC_VERTEX addVertex(Vec3 p, Vec3 n, float tu, float tv) {
		STATIC_VERTEX v;
		Frame frame;
		frame.fromVector(n);

		v.pos = p;
		v.normal = n;
		v.tangent = frame.u;
		v.tu = tu;
		v.tv = tv;
		return v;
	}

	void initialize(Core* core, PSOManager* psos, ShaderManager* shaders) {
		vertices.push_back(addVertex(Vec3(-1.f, 0.f, -1.f), Vec3(0.f, 1.f, 0.f), 0.f, 0.f));
		vertices.push_back(addVertex(Vec3(1.f, 0.f, -1.f), Vec3(0.f, 1.f, 0.f), 1.f, 0.f));
		vertices.push_back(addVertex(Vec3(-1.f, 0.f, 1.f), Vec3(0.f, 1.f, 0.f), 0.f, 1.f));
		vertices.push_back(addVertex(Vec3(1.f, 0.f, 1.f), Vec3(0.f, 1.f, 0.f), 1.f, 1.f));

		indices.push_back(2); indices.push_back(1); indices.push_back(0);
		indices.push_back(1); indices.push_back(2); indices.push_back(3);

		mesh.initialize(core, vertices, indices);
		shaders->loadShader(core, "Plane", "VertexShaderStatic.txt", "PixelShader.txt");
		shadername = "Plane";
		psos->createPSO(core, "PlanePSO", shaders->getShader(shadername)->vertexShader, shaders->getShader(shadername)->pixelShader, VertexLayoutCache::getStaticLayout());
	}

	void draw(Core* core, PSOManager* psos, ShaderManager* shaders, Matrix& vp, Matrix& w) {
		psos->bind(core, "PlanePSO");
		shaders->updateConstantVertexShaderBuffer("Plane", "staticMeshBuffer", "W", &w);
		shaders->updateConstantVertexShaderBuffer("Plane", "staticMeshBuffer", "VP", &vp);
		shaders->apply(core, shadername);
		mesh.draw(core);
	}
};