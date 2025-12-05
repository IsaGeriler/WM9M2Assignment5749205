#include <d3d12shader.h>
#include <fstream>
#include <sstream>
#include <unordered_map>

#include "Core.h"
#include "GamesEngineeringBase.h"
#include "MyMath.h"
#include "Window.h"

#pragma comment(lib, "dxguid.lib")

const int WIDTH = 1024;
const int HEIGHT = 1024;

extern "C" {
	_declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
}

struct PRIM_VERTEX {
	Vec3 position;
	Colour colour;
};

class Mesh {
public:
	// Create buffer and upload vertices to GPU
	ID3D12Resource* vertexBuffer;

	// Create view member variable
	D3D12_VERTEX_BUFFER_VIEW vbView;

	// Define layout
	D3D12_INPUT_ELEMENT_DESC inputLayout[2];
	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc;

	~Mesh() { vertexBuffer->Release(); }

	void initialize(Core* core, void* vertices, int vertexSizeInBytes, int numVertices) {
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

		// Allocate memory
		core->device->CreateCommittedResource(&heapprops, D3D12_HEAP_FLAG_NONE, &vbDesc, D3D12_RESOURCE_STATE_COMMON, NULL, IID_PPV_ARGS(&vertexBuffer));

		// Copy vertices using our helper function
		core->uploadResource(vertexBuffer, vertices, numVertices * vertexSizeInBytes, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

		// Fill in view in helper function
		vbView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
		vbView.StrideInBytes = vertexSizeInBytes;
		vbView.SizeInBytes = numVertices * vertexSizeInBytes;

		// Fill in layout
		inputLayout[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
		inputLayout[1] = { "COLOUR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
		inputLayoutDesc.NumElements = 2;
		inputLayoutDesc.pInputElementDescs = inputLayout;
	}

	void draw(Core* core) {
		core->getCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		core->getCommandList()->IASetVertexBuffers(0, 1, &vbView);
		core->getCommandList()->DrawInstanced(3, 1, 0, 0);
	}
};

class PSOManager {
public:
	std::unordered_map<std::string, ID3D12PipelineState*> psos;

	~PSOManager() { for (auto& pso : psos) pso.second->Release(); }

	void createPSO(Core* core, std::string name, ID3DBlob* vs, ID3DBlob* ps, D3D12_INPUT_LAYOUT_DESC layout) {
		// Avoid creating extra state
		if (psos.find(name) != psos.end()) return;

		// Configure GPU pipeline with shaders, layout and Root Signature
		D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = {};
		desc.InputLayout = layout;
		desc.pRootSignature = core->rootSignature;
		desc.VS = { vs->GetBufferPointer(), vs->GetBufferSize() };
		desc.PS = { ps->GetBufferPointer(), ps->GetBufferSize() };

		// Responsible for configuring the rasterizer
		D3D12_RASTERIZER_DESC rasterDesc = {};
		rasterDesc.FillMode = D3D12_FILL_MODE_SOLID;
		rasterDesc.CullMode = D3D12_CULL_MODE_NONE;
		rasterDesc.FrontCounterClockwise = FALSE;
		rasterDesc.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
		rasterDesc.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
		rasterDesc.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
		rasterDesc.DepthClipEnable = TRUE;
		rasterDesc.MultisampleEnable = FALSE;
		rasterDesc.AntialiasedLineEnable = FALSE;
		rasterDesc.ForcedSampleCount = 0;
		rasterDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
		desc.RasterizerState = rasterDesc;

		// Responsible for configuring the depth buffer
		D3D12_DEPTH_STENCIL_DESC depthStencilDesc = {};
		depthStencilDesc.DepthEnable = TRUE;
		depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
		depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
		depthStencilDesc.StencilEnable = FALSE;
		desc.DepthStencilState = depthStencilDesc;

		// Blend State
		D3D12_BLEND_DESC blendDesc = {};
		blendDesc.AlphaToCoverageEnable = FALSE;
		blendDesc.IndependentBlendEnable = FALSE;
		const D3D12_RENDER_TARGET_BLEND_DESC defaultRenderTargetBlend = {
			FALSE, FALSE, D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
			D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD, D3D12_LOGIC_OP_NOOP, D3D12_COLOR_WRITE_ENABLE_ALL
		};

		for (int i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; i++) blendDesc.RenderTarget[i] = defaultRenderTargetBlend;
		desc.BlendState = blendDesc;

		// Render Target State and Topology
		desc.SampleMask = UINT_MAX;
		desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		desc.NumRenderTargets = 1;
		desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
		desc.SampleDesc.Count = 1;

		// Create Pipeline State Object
		ID3D12PipelineState* pso;
		HRESULT hr = core->device->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(&pso));

		// Insert into map
		psos.insert({ name, pso });
	}

	// Functionality to bind based on name
	void bind(Core* core, std::string name) { core->getCommandList()->SetPipelineState(psos[name]); }
};

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
		heapprops.Type = D3D12_HEAP_TYPE_UPLOAD;
		heapprops.CreationNodeMask = 1;
		heapprops.VisibleNodeMask = 1;
		D3D12_RESOURCE_DESC cbDesc = {};
		cbDesc.Width = cbSizeInBytesAligned;
		cbDesc.Height = 1;
		cbDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		cbDesc.DepthOrArraySize = 1;
		cbDesc.MipLevels = 1;
		cbDesc.SampleDesc.Count = 1;
		cbDesc.SampleDesc.Quality = 0;
		cbDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		core->device->CreateCommittedResource(&heapprops, D3D12_HEAP_FLAG_NONE, &cbDesc, D3D12_RESOURCE_STATE_GENERIC_READ, NULL, IID_PPV_ARGS(&constantBuffer));
		constantBuffer->Map(0, NULL, (void**)&buffer);
	}

	void update(std::string name, void* data) {
		ConstantBufferVariable cbVariable = constantBufferData[name];
		unsigned int offset = offsetIndex * cbSizeInBytes;
		memcpy(&buffer[offset + cbVariable.offset], data, cbVariable.size);
	}

	D3D12_GPU_VIRTUAL_ADDRESS getGPUAddress() const { return (constantBuffer->GetGPUVirtualAddress() + (offsetIndex * cbSizeInBytes)); }

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
	void loadShader(Core* core, std::string shadername, std::string psfilename, std::string vsfilename) {
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

// Primitive Class
class ScreenSpaceTriangle {
public:
	PRIM_VERTEX vertices[3];
	Mesh mesh;
	std::string shaderName;

	Vec4 lights[4];
	float time = 0.f;
	GamesEngineeringBase::Timer timer;

	void initialize(Core* core, PSOManager* psos, ShaderManager* shaders) {
		vertices[0].position = Vec3(0, 1.0f, 0);
		vertices[0].colour = Colour(0, 1.0f, 0);
		vertices[1].position = Vec3(-1.0f, -1.0f, 0);
		vertices[1].colour = Colour(1.0f, 0, 0);
		vertices[2].position = Vec3(1.0f, -1.0f, 0);
		vertices[2].colour = Colour(0, 0, 1.0f);
		mesh.initialize(core, &vertices[0], sizeof(PRIM_VERTEX), 3);
		shaderName = "Triangle";
		shaders->loadShader(core, shaderName, "TrianglePS.txt", "TriangleVS.txt");
		psos->createPSO(core, shaderName, shaders->getShader(shaderName)->vertexShader, shaders->getShader(shaderName)->pixelShader, mesh.inputLayoutDesc);
	}

	void draw(Core* core, PSOManager* psos, ShaderManager* shaders) {
		time += timer.dt();

		for (int i = 0; i < 4; i++) {
			float angle = time + (i * M_PI / 2.0f);
			lights[i] = Vec4(
				WIDTH / 2.0f + (cosf(angle) * (WIDTH * 0.3f)),    // x
				HEIGHT / 2.0f + (sinf(angle) * (HEIGHT * 0.3f)),  // y
				0, 0											  // z and w
			);
		}

		shaders->apply(core, shaderName);
		shaders->updateConstantPixelShaderBuffer(shaderName, "timebuffer", "time", &time);
		shaders->updateConstantPixelShaderBuffer(shaderName, "lightbuffer", "lights", lights);
		psos->bind(core, "Triangle");
		mesh.draw(core);
	}
};

int WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ PSTR lpCmdLine, _In_ int nCmdShow) {
	Core core;
	Window window;
	PSOManager psos;
	ShaderManager shaders;
	ScreenSpaceTriangle primitive;

	window.initialize(WIDTH, HEIGHT, "MyWindow");
	core.initialize(window.hwnd, WIDTH, HEIGHT);
	primitive.initialize(&core, &psos, &shaders);

	while (true) {
		core.beginFrame();
		window.processMessages();
		if (window.keys[VK_ESCAPE] == 1) break;
		core.beginRenderPass();
		primitive.draw(&core, &psos, &shaders);
		core.finishFrame();
	}
	core.flushGraphicsQueue();
	return 0;
}