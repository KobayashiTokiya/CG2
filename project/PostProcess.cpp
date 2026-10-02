#include "PostProcess.h"
#include <cassert>

void PostProcess::Initialize(DirectXCommon* dxCommon)
{
	ID3D12Device* device = dxCommon->GetDevice();
	if (!device)
	{
		Logger::Log("PostProcess::Initialize - Device is nullptr!\n");
		return;
	}

	// =========================================================
	// 1. ルートシグネチャ (RootSignature) の作成
	// =========================================================
	D3D12_DESCRIPTOR_RANGE descriptorRanges[1] = {};
	descriptorRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRanges[0].NumDescriptors = 1;
	descriptorRanges[0].BaseShaderRegister = 0; // t0
	descriptorRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_ROOT_PARAMETER rootParameters[2] = {};
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[0].DescriptorTable.NumDescriptorRanges = _countof(descriptorRanges);
	rootParameters[0].DescriptorTable.pDescriptorRanges = descriptorRanges;

	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[1].Descriptor.ShaderRegister = 0; // b0

	D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
	staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].ShaderRegister = 0; // s0
	staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc{};
	rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;
	rootSignatureDesc.pParameters = rootParameters;
	rootSignatureDesc.NumParameters = _countof(rootParameters);
	rootSignatureDesc.pStaticSamplers = staticSamplers;
	rootSignatureDesc.NumStaticSamplers = _countof(staticSamplers);

	Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;

	HRESULT hr = D3D12SerializeRootSignature(
		&rootSignatureDesc,
		D3D_ROOT_SIGNATURE_VERSION_1,
		signatureBlob.GetAddressOf(),
		errorBlob.GetAddressOf()
	);
	if (FAILED(hr))
	{
		Logger::Log("Failed to serialize RootSignature for PostProcess!\n");
		assert(false);
		return;
	}

	hr = device->CreateRootSignature(
		0,
		signatureBlob->GetBufferPointer(),
		signatureBlob->GetBufferSize(),
		IID_PPV_ARGS(&rootSignature_)
	);
	if (FAILED(hr))
	{
		Logger::Log("Failed to create RootSignature for PostProcess!\n");
		assert(false);
		return;
	}

	// =========================================================
	// 2. シェーダーのコンパイル
	// =========================================================
	Microsoft::WRL::ComPtr vsBlob = dxCommon->CompileShader(L"Resource/shaders/Fullscreen.VS.hlsl", L"vs_6_5");
	Microsoft::WRL::ComPtr psBlobPostProcess = dxCommon->CompileShader(L"Resource/shaders/PostProcess.PS.hlsl", L"ps_6_5");
	Microsoft::WRL::ComPtr psBlobGrayscale = dxCommon->CompileShader(L"Resource/shaders/Grayscale.PS.hlsl", L"ps_6_5");
	Microsoft::WRL::ComPtr psBlobVignette = dxCommon->CompileShader(L"Resource/shaders/Vignette.PS.hlsl", L"ps_6_5");
	Microsoft::WRL::ComPtr psBlobBoxFilter = dxCommon->CompileShader(L"Resource/shaders/BoxFilter.PS.hlsl", L"ps_6_5");
	Microsoft::WRL::ComPtr psBlobGaussian = dxCommon->CompileShader(L"Resource/shaders/GaussianFilter.PS.hlsl", L"ps_6_5");

	// シェーダーが1つでも読み込めなかった場合は即座に中断
	if (!vsBlob || !psBlobPostProcess || !psBlobGrayscale || !psBlobVignette || !psBlobBoxFilter || !psBlobGaussian)
	{
		Logger::Log("PostProcess Shader Compilation Failed!\n");
		assert(false && "シェーダーファイルの読み込みまたはコンパイルに失敗しました。");
		return;
	}

	// =========================================================
	// 3. パイプライン状態オブジェクト (PSO) の作成
	// =========================================================
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
	psoDesc.pRootSignature = rootSignature_.Get();
	psoDesc.VS = { vsBlob->GetBufferPointer(), vsBlob->GetBufferSize() };
	psoDesc.InputLayout.pInputElementDescs = nullptr;
	psoDesc.InputLayout.NumElements = 0;

	psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	psoDesc.RasterizerState.FrontCounterClockwise = FALSE;
	psoDesc.RasterizerState.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
	psoDesc.RasterizerState.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
	psoDesc.RasterizerState.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
	psoDesc.RasterizerState.DepthClipEnable = TRUE;
	psoDesc.RasterizerState.MultisampleEnable = FALSE;
	psoDesc.RasterizerState.AntialiasedLineEnable = FALSE;
	psoDesc.RasterizerState.ForcedSampleCount = 0;
	psoDesc.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	psoDesc.BlendState.AlphaToCoverageEnable = FALSE;
	psoDesc.BlendState.IndependentBlendEnable = FALSE;
	for (UINT i = 0; i < 8; ++i)
	{
		psoDesc.BlendState.RenderTarget[i].BlendEnable = FALSE;
		psoDesc.BlendState.RenderTarget[i].LogicOpEnable = FALSE;
		psoDesc.BlendState.RenderTarget[i].SrcBlend = D3D12_BLEND_ONE;
		psoDesc.BlendState.RenderTarget[i].DestBlend = D3D12_BLEND_ZERO;
		psoDesc.BlendState.RenderTarget[i].BlendOp = D3D12_BLEND_OP_ADD;
		psoDesc.BlendState.RenderTarget[i].SrcBlendAlpha = D3D12_BLEND_ONE;
		psoDesc.BlendState.RenderTarget[i].DestBlendAlpha = D3D12_BLEND_ZERO;
		psoDesc.BlendState.RenderTarget[i].BlendOpAlpha = D3D12_BLEND_OP_ADD;
		psoDesc.BlendState.RenderTarget[i].LogicOp = D3D12_LOGIC_OP_NOOP;
		psoDesc.BlendState.RenderTarget[i].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	}

	psoDesc.DepthStencilState.DepthEnable = FALSE;
	psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_NEVER;
	psoDesc.DepthStencilState.StencilEnable = FALSE;

	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.SampleDesc.Count = 1;
	psoDesc.SampleDesc.Quality = 0;

	psoDesc.PS = { psBlobPostProcess->GetBufferPointer(), psBlobPostProcess->GetBufferSize() };
	hr = device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipelineStatePostProcess_));
	assert(SUCCEEDED(hr));

	psoDesc.PS = { psBlobGrayscale->GetBufferPointer(), psBlobGrayscale->GetBufferSize() };
	hr = device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipelineStateGrayscale_));
	assert(SUCCEEDED(hr));

	psoDesc.PS = { psBlobVignette->GetBufferPointer(), psBlobVignette->GetBufferSize() };
	hr = device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipelineStateVignette_));
	assert(SUCCEEDED(hr));

	psoDesc.PS = { psBlobBoxFilter->GetBufferPointer(), psBlobBoxFilter->GetBufferSize() };
	hr = device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipelineStateBoxFilter_));
	assert(SUCCEEDED(hr));

	psoDesc.PS = { psBlobGaussian->GetBufferPointer(), psBlobGaussian->GetBufferSize() };
	hr = device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipelineStateGaussian_));
	assert(SUCCEEDED(hr));

	// =========================================================
	// 4. 定数バッファの生成とマッピング
	// =========================================================
	uint32_t sizeCB = (sizeof(PostProcessData) + 0xFF) & ~0xFF; // 256バイトアライメント[cite: 19]
	D3D12_HEAP_PROPERTIES heapProps{};
	heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;

	D3D12_RESOURCE_DESC resDesc{};
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resDesc.Width = sizeCB;
	resDesc.Height = 1;
	resDesc.DepthOrArraySize = 1;
	resDesc.MipLevels = 1;
	resDesc.Format = DXGI_FORMAT_UNKNOWN;
	resDesc.SampleDesc.Count = 1;
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	hr = device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &resDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&constBuffer_));
	if (FAILED(hr) || !constBuffer_)
	{
		Logger::Log("Failed to create ConstantBuffer for PostProcess!\n");
		assert(false);
		return;
	}

	// 常時マップ状態にする
	hr = constBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&cBufferData_));
	if (FAILED(hr) || !cBufferData_)
	{
		Logger::Log("Failed to Map ConstantBuffer for PostProcess!\n");
		assert(false);
		return;
	}
}

void PostProcess::Draw(ID3D12GraphicsCommandList* commandList, RenderTexture* renderTexture, bool enable, int effectModel, const Vector3& colorScale)
{
	// 安全対策：Initialize が失敗していた場合は描画処理をスキップ
	if (!cBufferData_ || !constBuffer_)
	{
		return;
	}

	cBufferData_->enable = enable ? 1 : 0;
	cBufferData_->effectMode = effectModel;
	cBufferData_->colorScale = colorScale;

	commandList->SetGraphicsRootSignature(rootSignature_.Get());

	if (!enable)
	{
		commandList->SetPipelineState(pipelineStatePostProcess_.Get());
	}
	else
	{
		switch (effectModel)
		{
		case 0: commandList->SetPipelineState(pipelineStatePostProcess_.Get()); break;
		case 1: commandList->SetPipelineState(pipelineStateGrayscale_.Get()); break;
		case 2: commandList->SetPipelineState(pipelineStateVignette_.Get()); break;
		case 3: commandList->SetPipelineState(pipelineStateBoxFilter_.Get()); break;
		case 4: commandList->SetPipelineState(pipelineStateGaussian_.Get()); break;
		default: commandList->SetPipelineState(pipelineStatePostProcess_.Get()); break;
		}
	}

	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->SetGraphicsRootConstantBufferView(1, constBuffer_->GetGPUVirtualAddress());
	commandList->SetGraphicsRootDescriptorTable(0, renderTexture->GetSrvHandle());
	commandList->DrawInstanced(3, 1, 0, 0);
}