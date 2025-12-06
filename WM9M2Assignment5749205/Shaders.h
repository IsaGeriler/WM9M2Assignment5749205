#pragma once

#include <d3d12.h>
#include <d3dcompiler.h>
#include <map>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>

#include "Core.h"

#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "d3dcompiler.lib")

struct ConstantBufferVariable {
	unsigned int offset;
	unsigned int size;
};

class ConstantBuffer {
public:
	// Create resource to store constant buffer (and CPU memory, and size)
	ID3D12Resource* constantBuffer;
	unsigned char* buffer;
	unsigned int cbSizeInBytes;

	unsigned int numInstances;
	unsigned int offsetIndex;

	std::string name;
	std::map<std::string, ConstantBufferVariable> constantBufferData;

	void initialize(Core* core, unsigned int sizeInBytes, unsigned int _maxDrawCalls = 1024) {
		cbSizeInBytes = (sizeInBytes + 255) & ~255;
		unsigned int cbSizeInBytesAligned = cbSizeInBytes * _maxDrawCalls;
		numInstances = _maxDrawCalls;
		offsetIndex = 0;
		HRESULT hr;
		D3D12_HEAP_PROPERTIES heapprops = {};
		memset(&heapprops, 0, sizeof(D3D12_HEAP_PROPERTIES));
		heapprops.Type = D3D12_HEAP_TYPE_UPLOAD;
		heapprops.CreationNodeMask = 1;
		heapprops.VisibleNodeMask = 1;
		D3D12_RESOURCE_DESC cbDesc = {};
		memset(&cbDesc, 0, sizeof(D3D12_RESOURCE_DESC));
		cbDesc.Width = cbSizeInBytesAligned;
		cbDesc.Height = 1;
		cbDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		cbDesc.DepthOrArraySize = 1;
		cbDesc.MipLevels = 1;
		cbDesc.SampleDesc.Count = 1;
		cbDesc.SampleDesc.Quality = 0;
		cbDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		hr = core->device->CreateCommittedResource(&heapprops, D3D12_HEAP_FLAG_NONE, &cbDesc, D3D12_RESOURCE_STATE_GENERIC_READ, NULL, __uuidof(ID3D12Resource), (void**)&constantBuffer);
		D3D12_RANGE readRange = { 0, 0 };
		hr = constantBuffer->Map(0, &readRange, (void**)&buffer);
	}

	void update(std::string name, void* data) {
		ConstantBufferVariable cbVariable = constantBufferData[name];
		unsigned int offset = offsetIndex * cbSizeInBytes;
		memcpy(&buffer[offset + cbVariable.offset], data, cbVariable.size);
	}

	D3D12_GPU_VIRTUAL_ADDRESS getGPUAddress() const {
		return (constantBuffer->GetGPUVirtualAddress() + (offsetIndex * cbSizeInBytes));
	}

	// Ring buffer
	void next() {
		offsetIndex++;
		if (offsetIndex >= numInstances) offsetIndex = 0;
	}

	void free() {
		constantBuffer->Unmap(0, NULL);
		constantBuffer->Release();
	}
};

// Shader
class Shader {
public:
	// Vertex and Pixel Shaders
	ID3DBlob* vertexShader;
	ID3DBlob* pixelShader;

	std::vector<ConstantBuffer> psConstantBuffers;
	std::vector<ConstantBuffer> vsConstantBuffers;

	void initializeConstantBuffers(Core* core, ID3DBlob* shader, std::vector<ConstantBuffer>& buffers) {
		// Reflect shader and get details (description)
		ID3D12ShaderReflection* reflection;
		D3DReflect(shader->GetBufferPointer(), shader->GetBufferSize(), IID_PPV_ARGS(&reflection));
		D3D12_SHADER_DESC desc;
		reflection->GetDesc(&desc);

		//  Iterate over constant buffers
		for (int i = 0; i < desc.ConstantBuffers; i++) {
			// Get details about i’th constant buffer
			ConstantBuffer buffer;
			ID3D12ShaderReflectionConstantBuffer* constantBuffer = reflection->GetConstantBufferByIndex(i);
			D3D12_SHADER_BUFFER_DESC cbDesc;
			constantBuffer->GetDesc(&cbDesc);
			buffer.name = cbDesc.Name;
			unsigned int totalSize = 0;

			// Iterate over variables in constant buffer
			for (int j = 0; j < cbDesc.Variables; j++) {
				// Fill in details for each variable and keep a running total of size
				ID3D12ShaderReflectionVariable* var = constantBuffer->GetVariableByIndex(j);
				D3D12_SHADER_VARIABLE_DESC vDesc;
				var->GetDesc(&vDesc);
				ConstantBufferVariable bufferVariable;
				bufferVariable.offset = vDesc.StartOffset;
				bufferVariable.size = vDesc.Size;
				buffer.constantBufferData.insert({ vDesc.Name, bufferVariable });
				totalSize += bufferVariable.size;
			}
			buffer.initialize(core, totalSize);
			buffers.push_back(buffer);
		}
		reflection->Release();
	}

	void loadPixelShaders(Core* core, std::string pixelShaderStr) {
		ID3DBlob* status;
		HRESULT hr = D3DCompile(pixelShaderStr.c_str(), strlen(pixelShaderStr.c_str()), NULL, NULL, NULL, "PS", "ps_5_0", 0, 0, &pixelShader, &status);

		if (FAILED(hr)) {
			printf("%s\n", (char*)status->GetBufferPointer());
			exit(0);
		}
		initializeConstantBuffers(core, pixelShader, psConstantBuffers);
	}

	void loadVertexShaders(Core* core, std::string vertexShaderStr) {
		ID3DBlob* status;
		HRESULT hr = D3DCompile(vertexShaderStr.c_str(), strlen(vertexShaderStr.c_str()), NULL, NULL, NULL, "VS", "vs_5_0", 0, 0, &vertexShader, &status);

		if (FAILED(hr)) {
			printf("%s\n", (char*)status->GetBufferPointer());
			exit(0);
		}
		initializeConstantBuffers(core, vertexShader, vsConstantBuffers);
	}

	void apply(Core* core) {
		for (int i = 0; i < vsConstantBuffers.size(); i++) {
			core->getCommandList()->SetGraphicsRootConstantBufferView(0, vsConstantBuffers[i].getGPUAddress());
			vsConstantBuffers[i].next();
		}

		for (int i = 0; i < psConstantBuffers.size(); i++) {
			core->getCommandList()->SetGraphicsRootConstantBufferView(1, psConstantBuffers[i].getGPUAddress());
			psConstantBuffers[i].next();
		}
	}

	void updateConstantBuffer(std::string constantBufferName, std::string variableName, void* data, std::vector<ConstantBuffer>& buffers) {
		for (int i = 0; i < buffers.size(); i++) {
			if (buffers[i].name == constantBufferName) {
				buffers[i].update(variableName, data);
				return;
			}
		}
	}

	void updateConstantPixelShaderBuffer(std::string constantBufferName, std::string variableName, void* data) { updateConstantBuffer(constantBufferName, variableName, data, psConstantBuffers); }
	void updateConstantVertexShaderBuffer(std::string constantBufferName, std::string variableName, void* data) { updateConstantBuffer(constantBufferName, variableName, data, vsConstantBuffers); }

	void free() {
		pixelShader->Release();
		vertexShader->Release();

		for (auto constantBuffer : psConstantBuffers) constantBuffer.free();
		for (auto constantBuffer : vsConstantBuffers) constantBuffer.free();
	}
};

class ShaderManager {
public:
	std::map<std::string, Shader> shaders;

	~ShaderManager() {
		for (auto shader = shaders.begin(); shader != shaders.end();) {
			shader->second.free();
			shaders.erase(shader++);
		}
	}

	// Read Shader
	std::string readShader(std::string filename) {
		std::ifstream file(filename);
		std::stringstream buffer;
		buffer << file.rdbuf();
		return buffer.str();
	}

	// Load Shader
	void loadShader(Core* core, std::string shadername, std::string vsfilename, std::string psfilename) {
		std::map<std::string, Shader>::iterator iter = shaders.find(shadername);
		if (iter != shaders.end()) return;

		Shader shader;
		shader.loadPixelShaders(core, readShader(psfilename));
		shader.loadVertexShaders(core, readShader(vsfilename));
		shaders.insert({ shadername, shader });
	}

	// Get Shader
	Shader* getShader(std::string shadername) { return &shaders[shadername]; }

	// Apply
	void apply(Core* core, std::string shadername) { shaders[shadername].apply(core); }

	// Update Constant PixelShader Buffer
	void updateConstantPixelShaderBuffer(std::string shadername, std::string constantBufferName, std::string variableName, void* data) { shaders[shadername].updateConstantPixelShaderBuffer(constantBufferName, variableName, data); }

	// Update Constant VertexShader Buffer
	void updateConstantVertexShaderBuffer(std::string shadername, std::string constantBufferName, std::string variableName, void* data) { shaders[shadername].updateConstantVertexShaderBuffer(constantBufferName, variableName, data); }
};