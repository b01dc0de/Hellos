#include "Graphics_DX11.h"

#define DXCHECK(Result) if (FAILED(Result)) { return -1; }
#define DXCHECKMSG(Result, Msg) if (FAILED(Result)) { OutputDebugStringA((Msg)); return -1; }

namespace Graphics_DX11
{
	IDXGISwapChain* DX_SwapChain = nullptr;
	ID3D11Device* DX_Device = nullptr;
	D3D_FEATURE_LEVEL UsedFeatureLevel;
	ID3D11DeviceContext* DX_ImmediateContext = nullptr;

	ID3D11Texture2D* DX_BackBuffer = nullptr;
	ID3D11RenderTargetView* DX_RenderTargetView = nullptr;

	IDXGIFactory1* DX_Factory = nullptr;
	std::vector<IDXGIAdapter*> DX_AdapterList;
	std::vector<IDXGIOutput*> DX_Outputs;
	std::vector<DXGI_MODE_DESC*> OutputModeDescList;

	ID3D11RasterizerState* DX_RasterizerState = nullptr;
	ID3D11Texture2D* DX_DepthStencil = nullptr;
	ID3D11DepthStencilView* DX_DepthStencilView = nullptr;
	ID3D11BlendState* DX_BlendState = nullptr;

	ID3D11Buffer* DX_VertexBuffer = nullptr;
	ID3D11Buffer* DX_IndexBuffer = nullptr;

	ID3D11VertexShader* DX_VertexShader = nullptr;
	ID3D11PixelShader* DX_PixelShader = nullptr;
	ID3D11InputLayout* DX_InputLayout = nullptr;

	int CompileShaderHelper(LPCWSTR SourceFileName, LPCSTR EntryPointFunction, LPCSTR Profile, ID3DBlob** ShaderBlob)
	{
		HRESULT Result = S_OK;

		if (SourceFileName == nullptr || EntryPointFunction == nullptr || Profile == nullptr || ShaderBlob == nullptr)
		{
			return E_INVALIDARG;
		}

		*ShaderBlob = nullptr;

		UINT CompileFlags = D3DCOMPILE_ENABLE_STRICTNESS;
	#if _DEBUG
		CompileFlags |= D3DCOMPILE_DEBUG;
		CompileFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
	#endif

		ID3DBlob* OutBlob = nullptr;
		ID3DBlob* ErrorMsgBlob = nullptr;

		Result = D3DCompileFromFile
		(
			SourceFileName,
			nullptr,
			D3D_COMPILE_STANDARD_FILE_INCLUDE,
			EntryPointFunction,
			Profile,
			CompileFlags,
			0, //UINT Flags2
			&OutBlob,
			&ErrorMsgBlob
		);

		if (FAILED(Result) && OutBlob)
		{
			OutBlob->Release();
			OutBlob = nullptr;
		}
		if (ErrorMsgBlob)
		{
			OutputDebugStringA((char*)ErrorMsgBlob->GetBufferPointer());
			ErrorMsgBlob->Release();
		}

		*ShaderBlob = OutBlob;

		return Result;
	};

	VertexColor Vertices[] =
	{
		{{0.0f, 0.5f, 0.5f, 1.0f}, {0.0f, 0.0f, 1.0f, 1.0f}},
		{{0.5f, -0.5f, 0.5f, 1.0f}, {1.0f, 0.0f, 0.0f, 1.0f}},
		{{-0.5f, -0.5f, 0.5f, 1.0f}, {0.0f, 1.0f, 0.0f, 1.0f}}
	};

	int InitGraphics()
	{
		HRESULT Result = S_OK;

		D3D_FEATURE_LEVEL SupportedFeatureLevels[] =
		{
			D3D_FEATURE_LEVEL_11_1,
			D3D_FEATURE_LEVEL_11_0,
		};
		UINT NumSupportedFeatureLevels = ARRAYSIZE(SupportedFeatureLevels);
		D3D_FEATURE_LEVEL D3DFeatureLevel = D3D_FEATURE_LEVEL_11_0;
		(void)D3DFeatureLevel;

		CreateDXGIFactory1(__uuidof(IDXGIFactory), (void**)&DX_Factory);
		if (nullptr != DX_Factory)
		{
			IDXGIAdapter* DX_Adapter = nullptr;
			for (UINT AdapterIdx = 0; DX_Factory->EnumAdapters(AdapterIdx, &DX_Adapter); AdapterIdx++)
			{
				DX_AdapterList.push_back(DX_Adapter);
			}
		}

		for (int AdapterIdx = 0; AdapterIdx < DX_AdapterList.size(); AdapterIdx++)
		{
			IDXGIOutput* DX_Output = nullptr;
			for (int OutputIdx = 0; DX_AdapterList[AdapterIdx]->EnumOutputs(OutputIdx, &DX_Output) != DXGI_ERROR_NOT_FOUND; OutputIdx++)
			{
				DXGI_FORMAT Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
				UINT NumModes = 0;
				Result = DX_Output->GetDisplayModeList(Format, 0, &NumModes, nullptr);
				if (NumModes != 0)
				{
					DXGI_MODE_DESC* ModeList = new DXGI_MODE_DESC[NumModes];
					Result = DX_Output->GetDisplayModeList(Format, 0, &NumModes, ModeList);
					OutputModeDescList.push_back(ModeList);
				}
			}
		}

		DXGI_SAMPLE_DESC SharedSampleDesc = {};
		SharedSampleDesc.Count = 4;
		SharedSampleDesc.Quality = (UINT)D3D11_STANDARD_MULTISAMPLE_PATTERN;

		UINT FrameRefreshRate = 60;
		DXGI_SWAP_CHAIN_DESC swapchain_desc = {};
		swapchain_desc.BufferCount = 2;
		swapchain_desc.BufferDesc.Width = WinResX;
		swapchain_desc.BufferDesc.Height = WinResY;
		swapchain_desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapchain_desc.BufferDesc.RefreshRate.Numerator = FrameRefreshRate;
		swapchain_desc.BufferDesc.RefreshRate.Denominator = 1;
		swapchain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapchain_desc.OutputWindow = hWindow;
		swapchain_desc.SampleDesc = SharedSampleDesc;
		swapchain_desc.Windowed = true;

		UINT CreateDeviceFlags = 0;
	#ifdef _DEBUG
		CreateDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
	#endif

		Result = D3D11CreateDeviceAndSwapChain(
			nullptr,					//IDXGIAdapter* pAdapter
			D3D_DRIVER_TYPE_HARDWARE,	//D3D_DRIVER_TYPE DriverType
			nullptr,					//HMODULE Software
			CreateDeviceFlags,			//UINT Flags
			SupportedFeatureLevels,		//const D3D_FEATURE_LEVEL* pFeatureLevels
			NumSupportedFeatureLevels,	//UINT FeatureLevels
			D3D11_SDK_VERSION,			//UINT SDKVersion
			&swapchain_desc,			//const DXGI_SWAP_CHAIN_DESC* pSwapChainDesc
			&DX_SwapChain,
			&DX_Device,
			&UsedFeatureLevel,
			&DX_ImmediateContext
		);
		DXCHECK(Result);

		Result = DX_SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&DX_BackBuffer);
		DXCHECK(Result);

		Result = DX_Device->CreateRenderTargetView(DX_BackBuffer, nullptr, &DX_RenderTargetView);
		DXCHECK(Result);

		D3D11_RASTERIZER_DESC RasterDesc = {};
		RasterDesc.FillMode = D3D11_FILL_SOLID;
		RasterDesc.CullMode = D3D11_CULL_BACK;
		RasterDesc.FrontCounterClockwise = true;
		RasterDesc.DepthClipEnable = true;
		RasterDesc.ScissorEnable = false;
		RasterDesc.MultisampleEnable = true;
		RasterDesc.AntialiasedLineEnable = true;

		Result = DX_Device->CreateRasterizerState(&RasterDesc, &DX_RasterizerState);
		DXCHECK(Result);

		DX_ImmediateContext->RSSetState(DX_RasterizerState);

		D3D11_TEXTURE2D_DESC DepthDesc = {};
		DepthDesc.Width = WinResX;
		DepthDesc.Height = WinResY;
		DepthDesc.MipLevels = 1;
		DepthDesc.ArraySize = 1;
		DepthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		DepthDesc.SampleDesc = SharedSampleDesc;
		DepthDesc.Usage = D3D11_USAGE_DEFAULT;
		DepthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		DepthDesc.CPUAccessFlags = 0;
		DepthDesc.MiscFlags = 0;

		Result = DX_Device->CreateTexture2D(&DepthDesc, nullptr, &DX_DepthStencil);
		DXCHECK(Result);

		D3D11_DEPTH_STENCIL_VIEW_DESC DepthStencilViewDesc = {};
		DepthStencilViewDesc.Format = DepthStencilViewDesc.Format;
		DepthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
		DepthStencilViewDesc.Texture2D.MipSlice = 0;

		Result = DX_Device->CreateDepthStencilView(DX_DepthStencil, &DepthStencilViewDesc, &DX_DepthStencilView);
		DXCHECK(Result);

		DX_ImmediateContext->OMSetRenderTargets(1, &DX_RenderTargetView, DX_DepthStencilView);

		D3D11_RENDER_TARGET_BLEND_DESC RTVBlendDesc = {};
		RTVBlendDesc.BlendEnable = true;
		RTVBlendDesc.SrcBlend = D3D11_BLEND_SRC_ALPHA;
		RTVBlendDesc.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		RTVBlendDesc.BlendOp = D3D11_BLEND_OP_ADD;
		RTVBlendDesc.SrcBlendAlpha = D3D11_BLEND_ONE;
		RTVBlendDesc.DestBlendAlpha = D3D11_BLEND_ZERO;
		RTVBlendDesc.BlendOpAlpha = D3D11_BLEND_OP_ADD;
		RTVBlendDesc.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALPHA;

		D3D11_BLEND_DESC BlendDesc = {};
		BlendDesc.RenderTarget[0] = RTVBlendDesc;

		Result = DX_Device->CreateBlendState(&BlendDesc, &DX_BlendState);
		DXCHECK(Result);

		D3D11_VIEWPORT Viewport_Desc = {};
		Viewport_Desc.Width = (FLOAT)WinResX;
		Viewport_Desc.Height = (FLOAT)WinResY;
		Viewport_Desc.MinDepth = 0.0f;
		Viewport_Desc.MaxDepth = 1.0f;
		Viewport_Desc.TopLeftX = 0;
		Viewport_Desc.TopLeftY = 0;
		DX_ImmediateContext->RSSetViewports(1, &Viewport_Desc);

		D3D11_BUFFER_DESC VertexBufferDesc =
		{
			sizeof(VertexColor) * ARRAYSIZE(Vertices),
			D3D11_USAGE_DEFAULT,
			D3D11_BIND_VERTEX_BUFFER,
			0,
			0
		};
		D3D11_SUBRESOURCE_DATA VertexBufferInitData = { Vertices, 0, 0 };
		Result = DX_Device->CreateBuffer(&VertexBufferDesc, &VertexBufferInitData, &DX_VertexBuffer);
		DXCHECK(Result);

		UINT Indices[] = { 0, 2, 1 };
		D3D11_BUFFER_DESC IndexBufferDesc =
		{
			sizeof(Indices),
			D3D11_USAGE_DEFAULT,
			D3D11_BIND_INDEX_BUFFER,
			0,
			0
		};
		D3D11_SUBRESOURCE_DATA IndexBufferInitData = { Indices, 0, 0 };
		Result = DX_Device->CreateBuffer(&IndexBufferDesc, &IndexBufferInitData, &DX_IndexBuffer);
		DXCHECK(Result);

		ID3DBlob* VSCodeBlob = nullptr;
		ID3DBlob* PSCodeBlob = nullptr;

		Result = CompileShaderHelper(L"src/hlsl/BaseShader.hlsl", "VSMain", "vs_5_0", &VSCodeBlob);
		DXCHECKMSG(Result, "Failed to compile Vertex Shader! :(\n");

		Result = CompileShaderHelper(L"src/hlsl/BaseShader.hlsl", "PSMain", "ps_5_0", &PSCodeBlob);
		DXCHECKMSG(Result, "Failed to compile Pixel Shader! :(\n");

		if (VSCodeBlob && PSCodeBlob)
		{
			Result = DX_Device->CreateVertexShader(VSCodeBlob->GetBufferPointer(), VSCodeBlob->GetBufferSize(), nullptr, &DX_VertexShader);
			DXCHECKMSG(Result, "Device could not create vertex shader! :(\n");

			Result = DX_Device->CreatePixelShader(PSCodeBlob->GetBufferPointer(), PSCodeBlob->GetBufferSize(), nullptr, &DX_PixelShader);
			DXCHECKMSG(Result, "Device could not create pixel shader! :(\n");

			D3D11_INPUT_ELEMENT_DESC InputLayoutDesc[] =
			{
				{"POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				// { "TEXTURE", ... }
				{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			};
			UINT NumInputElements = ARRAYSIZE(InputLayoutDesc);

			Result = DX_Device->CreateInputLayout(InputLayoutDesc, NumInputElements, VSCodeBlob->GetBufferPointer(), VSCodeBlob->GetBufferSize(), &DX_InputLayout);
			DXCHECKMSG(Result, "Device could not create input layout! :(\n");
		}
		if (VSCodeBlob) { VSCodeBlob->Release(); }
		if (PSCodeBlob) { PSCodeBlob->Release(); }

		return Result;
	}

	void UpdateGraphicsState()
	{
		UINT Stride = sizeof(VertexColor);
		UINT Offset = 0;

		DX_ImmediateContext->IASetInputLayout(DX_InputLayout);
		DX_ImmediateContext->IASetVertexBuffers(0, 1, &DX_VertexBuffer, &Stride, &Offset);
		DX_ImmediateContext->IASetIndexBuffer(DX_IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
		DX_ImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		DX_ImmediateContext->VSSetShader(DX_VertexShader, nullptr, 0);
		DX_ImmediateContext->PSSetShader(DX_PixelShader, nullptr, 0);
	}

	void Draw()
	{
		float ClearColor[4] = { 0.125f, 0.175f, 0.3f, 1.0f };
		float fDepth = 1.0f;
		DX_ImmediateContext->ClearRenderTargetView(DX_RenderTargetView, ClearColor);
		DX_ImmediateContext->ClearDepthStencilView(DX_DepthStencilView, D3D11_CLEAR_DEPTH, fDepth, 0);

		UpdateGraphicsState();

		DX_ImmediateContext->DrawIndexed((UINT)1 * 3, 0u, 0u);

		DX_SwapChain->Present(0, 0);
	}
}
