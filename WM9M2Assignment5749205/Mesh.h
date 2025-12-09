#pragma once

#include <d3d12.h>
#include <vector>

#include "Core.h"
#include "MyMath.h"

struct STATIC_VERTEX {
	Vec3 pos;
	Vec3 normal;
	Vec3 tangent;
	float tu;
	float tv;
};

struct ANIMATED_VERTEX {
	Vec3 pos;
	Vec3 normal;
	Vec3 tangent;
	float tu;
	float tv;
	unsigned int bonesIDs[4];
	float boneWeights[4];
};

class VertexLayoutCache {
public:
	static const D3D12_INPUT_LAYOUT_DESC& getStaticLayout() {
		static const D3D12_INPUT_ELEMENT_DESC inputLayoutStatic[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }, };
		static const D3D12_INPUT_LAYOUT_DESC desc = { inputLayoutStatic, 4 };
		return desc;
	}

	static const D3D12_INPUT_LAYOUT_DESC& getAnimatedLayout() {
		static const D3D12_INPUT_ELEMENT_DESC inputLayoutAnimated[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "BONEIDS", 0, DXGI_FORMAT_R32G32B32A32_UINT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "BONEWEIGHTS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }, };
		static const D3D12_INPUT_LAYOUT_DESC desc = { inputLayoutAnimated, 6 };
		return desc;
	}
};

class Mesh {
public:
	// Create buffer and upload vertices to GPU
	ID3D12Resource* vertexBuffer;
	ID3D12Resource* indexBuffer;

	// Create view member variable
	D3D12_VERTEX_BUFFER_VIEW vbView;
	D3D12_INDEX_BUFFER_VIEW ibView;

	// Define layout
	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc;
	unsigned int numMeshIndices;

	~Mesh() {
		indexBuffer->Release();
		vertexBuffer->Release();
	}

	void initialize(Core* core, void* vertices, int vertexSizeInBytes, int numVertices, unsigned int* indices, int numIndices) {
		// Specify vertex buffer will be in GPU memory heap
		D3D12_HEAP_PROPERTIES heapprops = {};
		heapprops.Type = D3D12_HEAP_TYPE_DEFAULT;
		heapprops.CreationNodeMask = 1;
		heapprops.VisibleNodeMask = 1;

		// Create vertex buffer on heap
		D3D12_RESOURCE_DESC vbDesc = {};
		vbDesc.Width = numVertices * vertexSizeInBytes;
		vbDesc.Height = 1;
		vbDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		vbDesc.DepthOrArraySize = 1;
		vbDesc.MipLevels = 1;
		vbDesc.SampleDesc.Count = 1;
		vbDesc.SampleDesc.Quality = 0;
		vbDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		HRESULT hr;

		// Allocate memory
		hr = core->device->CreateCommittedResource(&heapprops, D3D12_HEAP_FLAG_NONE, &vbDesc, D3D12_RESOURCE_STATE_COMMON, NULL, IID_PPV_ARGS(&vertexBuffer));

		// Copy vertices using our helper function
		core->uploadResource(vertexBuffer, vertices, numVertices * vertexSizeInBytes, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

		// Add Index Buffer creation on GPU memory
		D3D12_RESOURCE_DESC ibDesc = {};
		ibDesc.Width = numIndices * sizeof(unsigned int);
		ibDesc.Height = 1;
		ibDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		ibDesc.DepthOrArraySize = 1;
		ibDesc.MipLevels = 1;
		ibDesc.SampleDesc.Count = 1;
		ibDesc.SampleDesc.Quality = 0;
		ibDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		
		hr = core->device->CreateCommittedResource(&heapprops, D3D12_HEAP_FLAG_NONE, &ibDesc, D3D12_RESOURCE_STATE_COMMON, NULL, IID_PPV_ARGS(&indexBuffer));
		core->uploadResource(indexBuffer, indices, numIndices * sizeof(unsigned int), D3D12_RESOURCE_STATE_INDEX_BUFFER);

		// Fill in view in helper function
		vbView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
		vbView.StrideInBytes = vertexSizeInBytes;
		vbView.SizeInBytes = numVertices * vertexSizeInBytes;

		// Create view to Index Buffer
		ibView.BufferLocation = indexBuffer->GetGPUVirtualAddress();
		ibView.Format = DXGI_FORMAT_R32_UINT;
		ibView.SizeInBytes = numIndices * sizeof(unsigned int);

		numMeshIndices = numIndices;
	}

	void initialize(Core* core, std::vector<STATIC_VERTEX> vertices, std::vector<unsigned int> indices) {
		initialize(core, &vertices[0], sizeof(STATIC_VERTEX), vertices.size(), &indices[0], indices.size());
		inputLayoutDesc = VertexLayoutCache::getStaticLayout();
	}

	void initialize(Core* core, std::vector<ANIMATED_VERTEX> vertices, std::vector<unsigned int> indices) {
		initialize(core, &vertices[0], sizeof(ANIMATED_VERTEX), vertices.size(), &indices[0], indices.size());
		inputLayoutDesc = VertexLayoutCache::getAnimatedLayout();
	}

	void draw(Core* core) {
		core->getCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		core->getCommandList()->IASetVertexBuffers(0, 1, &vbView);
		core->getCommandList()->IASetIndexBuffer(&ibView);
		core->getCommandList()->DrawIndexedInstanced(numMeshIndices, 1, 0, 0, 0);
	}
};