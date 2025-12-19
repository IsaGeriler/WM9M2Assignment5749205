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

struct INSTANCE_DATA {
	Matrix world;
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

	static const D3D12_INPUT_LAYOUT_DESC& getStaticInstancedLayout() {
		D3D12_INPUT_ELEMENT_DESC inputLayoutStaticInstanced[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "WORLD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 },
		{ "WORLD", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 },
		{ "WORLD", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 32, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 },
		{ "WORLD", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 48, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 }, };
		static const D3D12_INPUT_LAYOUT_DESC desc = { inputLayoutStaticInstanced, 8 };
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
	ID3D12Resource* instanceBuffer;

	// Create view member variable
	D3D12_VERTEX_BUFFER_VIEW vbView;
	D3D12_INDEX_BUFFER_VIEW ibView;
	D3D12_VERTEX_BUFFER_VIEW instanceView;

	// Define layout
	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc;
	unsigned int numMeshIndices;
	unsigned int numMeshInstances;

	// Is Instanced?
	bool isInstanced = false;

	~Mesh() {
		if (indexBuffer != nullptr) indexBuffer->Release();
		if (instanceBuffer != nullptr) instanceBuffer->Release();
		if (vertexBuffer != nullptr) vertexBuffer->Release();
	}

	// Initialize Meshes
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
		
		// Allocate memory
		hr = core->device->CreateCommittedResource(&heapprops, D3D12_HEAP_FLAG_NONE, &ibDesc, D3D12_RESOURCE_STATE_COMMON, NULL, IID_PPV_ARGS(&indexBuffer));
		core->uploadResource(indexBuffer, indices, numIndices * sizeof(unsigned int), D3D12_RESOURCE_STATE_INDEX_BUFFER);
		
		// Create view to Vertex Buffer
		vbView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
		vbView.StrideInBytes = vertexSizeInBytes;
		vbView.SizeInBytes = numVertices * vertexSizeInBytes;

		// Create view to Index Buffer
		ibView.BufferLocation = indexBuffer->GetGPUVirtualAddress();
		ibView.Format = DXGI_FORMAT_R32_UINT;
		ibView.SizeInBytes = numIndices * sizeof(unsigned int);

		numMeshIndices = numIndices;
	}

	// Initialize Instanced Meshes
	void initialize(Core* core, void* vertices, int vertexSizeInBytes, int numVertices, unsigned int* indices, int numIndices, void* instances, int numInstances, int instanceSizeInBytes) {
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

		// Allocate memory
		hr = core->device->CreateCommittedResource(&heapprops, D3D12_HEAP_FLAG_NONE, &ibDesc, D3D12_RESOURCE_STATE_COMMON, NULL, IID_PPV_ARGS(&indexBuffer));
		core->uploadResource(indexBuffer, indices, numIndices * sizeof(unsigned int), D3D12_RESOURCE_STATE_INDEX_BUFFER);

		// Create instance buffer on heap
		D3D12_RESOURCE_DESC instanceDesc = {};
		instanceDesc.Width = numInstances * instanceSizeInBytes;
		instanceDesc.Height = 1;
		instanceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		instanceDesc.DepthOrArraySize = 1;
		instanceDesc.MipLevels = 1;
		instanceDesc.SampleDesc.Count = 1;
		instanceDesc.SampleDesc.Quality = 0;
		instanceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

		// Allocate memory
		hr = core->device->CreateCommittedResource(&heapprops, D3D12_HEAP_FLAG_NONE, &instanceDesc, D3D12_RESOURCE_STATE_COMMON, NULL, IID_PPV_ARGS(&instanceBuffer));
		core->uploadResource(instanceBuffer, instances, numInstances * instanceSizeInBytes, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

		// Create view to Vertex Buffer
		vbView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
		vbView.StrideInBytes = vertexSizeInBytes;
		vbView.SizeInBytes = numVertices * vertexSizeInBytes;

		// Create view to Index Buffer
		ibView.BufferLocation = indexBuffer->GetGPUVirtualAddress();
		ibView.Format = DXGI_FORMAT_R32_UINT;
		ibView.SizeInBytes = numIndices * sizeof(unsigned int);

		// Create view to Instance Buffer
		instanceView.BufferLocation = instanceBuffer->GetGPUVirtualAddress();
		instanceView.StrideInBytes = instanceSizeInBytes;
		instanceView.SizeInBytes = numInstances * instanceSizeInBytes;

		numMeshIndices = numIndices;
		numMeshInstances = numInstances;
		isInstanced = true;
	}

	void initialize(Core* core, std::vector<STATIC_VERTEX> vertices, std::vector<unsigned int> indices) {
		initialize(core, &vertices[0], sizeof(STATIC_VERTEX), vertices.size(), &indices[0], indices.size());
		inputLayoutDesc = VertexLayoutCache::getStaticLayout();
	}

	void initialize(Core* core, std::vector<STATIC_VERTEX> vertices, std::vector<unsigned int> indices, std::vector<INSTANCE_DATA> instances) {
		initialize(core, &vertices[0], sizeof(STATIC_VERTEX), vertices.size(), &indices[0], indices.size(), &instances[0], instances.size(), sizeof(INSTANCE_DATA));
		inputLayoutDesc = VertexLayoutCache::getStaticInstancedLayout();
	}

	void initialize(Core* core, std::vector<ANIMATED_VERTEX> vertices, std::vector<unsigned int> indices) {
		initialize(core, &vertices[0], sizeof(ANIMATED_VERTEX), vertices.size(), &indices[0], indices.size());
		inputLayoutDesc = VertexLayoutCache::getAnimatedLayout();
	}

	void draw(Core* core) {
		if (isInstanced) {
			// Draw call for instanced meshes
			D3D12_VERTEX_BUFFER_VIEW bufferViews[2];
			bufferViews[0] = vbView;
			bufferViews[1] = instanceView;
			core->getCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			core->getCommandList()->IASetVertexBuffers(0, 2, bufferViews);
			core->getCommandList()->IASetIndexBuffer(&ibView);
			core->getCommandList()->DrawIndexedInstanced(numMeshIndices, numMeshInstances, 0, 0, 0);
		} else {
			// Draw call for meshes
			core->getCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			core->getCommandList()->IASetVertexBuffers(0, 1, &vbView);
			core->getCommandList()->IASetIndexBuffer(&ibView);
			core->getCommandList()->DrawIndexedInstanced(numMeshIndices, 1, 0, 0, 0);
		}
	}
};