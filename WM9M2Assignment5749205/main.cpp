#include <fstream>
#include <sstream>
#include <unordered_map>

#include "Core.h"
#include "MyMath.h"
#include "Window.h"

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

class ScreenSpaceTriangle {
public:
	PRIM_VERTEX vertices[3];
	Mesh mesh;

	void initialize(Core* core) {
		vertices[0].position = Vec3(0, 1.0f, 0);
		vertices[0].colour = Colour(0, 1.0f, 0);
		vertices[1].position = Vec3(-1.0f, -1.0f, 0);
		vertices[1].colour = Colour(1.0f, 0, 0);
		vertices[2].position = Vec3(1.0f, -1.0f, 0);
		vertices[2].colour = Colour(0, 0, 1.0f);
		mesh.initialize(core, &vertices[0], sizeof(PRIM_VERTEX), 3);
	}

	void draw(Core* core) {
		mesh.draw(core);
	}
};

class PSOManager {
public:
	std::unordered_map<std::string, ID3D12PipelineState*> psos;
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
	void bind(Core* core, std::string name) {
		core->getCommandList()->SetPipelineState(psos[name]);
	}
};

// Primitive
class Shader {
public:
	// Vertex and Pixel Shaders
	ID3DBlob* vertexShader;
	ID3DBlob* pixelShader;

	// Instance of ScreenSpaceTriangle
	ScreenSpaceTriangle triangle;

	// Instance of Pipeline Stage Object Manager
	PSOManager psos;

	std::string readShader(std::string filename) {
		std::ifstream file(filename);
		std::stringstream buffer;
		buffer << file.rdbuf();
		return buffer.str();
	}

	void compile(Core* core) {
		ID3DBlob* status;
		std::string vertexShaderStr = readShader("TriangleVS.txt");
		std::string pixelShaderStr = readShader("TrianglePS.txt");

		HRESULT hr;
		hr = D3DCompile(vertexShaderStr.c_str(), strlen(vertexShaderStr.c_str()), NULL, NULL, NULL, "VS", "vs_5_0", 0, 0, &vertexShader, &status);
		hr = D3DCompile(pixelShaderStr.c_str(), strlen(pixelShaderStr.c_str()), NULL, NULL, NULL, "PS", "ps_5_0", 0, 0, &pixelShader, &status);
		if (FAILED(hr)) (char*)status->GetBufferPointer();

		// Create pipeline stage
		psos.createPSO(core, "Triangle", vertexShader, pixelShader, triangle.mesh.inputLayoutDesc);
	}

	void initialize(Core* core) {
		triangle.initialize(core);
		compile(core);
	}

	void draw(Core* core) {
		psos.bind(core, "Triangle");
		triangle.draw(core);
	}
};

int WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ PSTR lpCmdLine, _In_ int nCmdShow) {
	Window window;
	Mesh mesh;
	Core core;
	Shader shader;

	window.initialize(WIDTH, HEIGHT, "My Window");
	core.initialize(window.hwnd, window.width, window.height);
	shader.initialize(&core);
	
	while (true) {
		if (window.keys[VK_ESCAPE] == 1) break;
		core.beginFrame();
		core.beginRenderPass();
		window.processMessages();
		shader.draw(&core);
		core.finishFrame();
	}
	core.flushGraphicsQueue();
	return 0;
}