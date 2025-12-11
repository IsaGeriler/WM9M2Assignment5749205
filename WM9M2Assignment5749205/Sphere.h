#pragma once

#include <vector>

#include "Core.h"
#include "Mesh.h"
#include "PSOManager.h"
#include "Shaders.h"
#include "Texture.h"

class Sphere {
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

	void initialize(Core* core, PSOManager* psos, TextureManager* textures, ShaderManager* shaders, int rings, int segments, float radius) {
		for (int lat = 0; lat <= rings; lat++) {
			float theta = lat * M_PI / rings;
			float sinTheta = sinf(theta);
			float cosTheta = cosf(theta);
			for (int lon = 0; lon <= segments; lon++) {
				float phi = lon * 2.0f * M_PI / segments;
				float sinPhi = sinf(phi);
				float cosPhi = cosf(phi);
				Vec3 position(radius * sinTheta * cosPhi, radius * cosTheta, radius * sinTheta * sinPhi);
				Vec3 normal = position.normalize();
				float tu = 1.0f - (float)lon / segments;
				float tv = 1.0f - (float)lat / rings;
				vertices.push_back(addVertex(position, normal, tu, tv));
			}
		}

		for (int lat = 0; lat < rings; lat++) {
			for (int lon = 0; lon < segments; lon++) {
				int current = lat * (segments + 1) + lon;
				int next = current + segments + 1;
				indices.push_back(current);
				indices.push_back(next);
				indices.push_back(current + 1);
				indices.push_back(current + 1);
				indices.push_back(next);
				indices.push_back(next + 1);
			}
		}
		textures->loadTexture(core, "SkyboxTexture", "Models/Textures/citrus_orchard_road_puresky.png");
		shaders->loadShader(core, "Sphere", "VertexShaderStatic.txt", "PixelShaderTextured.txt");
		mesh.initialize(core, vertices, indices);
		shadername = "Sphere";
		psos->createPSO(core, "SpherePSO", shaders->getShader(shadername)->vertexShader, shaders->getShader(shadername)->pixelShader, VertexLayoutCache::getStaticLayout());
	}

	void draw(Core* core, PSOManager* psos, TextureManager* textures, ShaderManager* shaders, Matrix& vp, Matrix& w) {
		psos->bind(core, "SpherePSO");
		shaders->updateConstantVertexShaderBuffer("Sphere", "staticMeshBuffer", "W", &w);
		shaders->updateConstantVertexShaderBuffer("Sphere", "staticMeshBuffer", "VP", &vp);
		shaders->apply(core, shadername);
		std::cout << "Models/Textures/citrus_orchard_road_puresky.png" << ' ' << textures->find("SkyboxTexture") << '\n';
		shaders->updateTexturePS(core, "Sphere", "tex", textures->find("SkyboxTexture"));
		mesh.draw(core);
	}
};