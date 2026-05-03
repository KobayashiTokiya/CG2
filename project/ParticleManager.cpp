#include "ParticleManager.h"
#include <d3d12.h>
#include<wrl.h>
#include "externals/imgui/imgui.h"
#include <string>

#include "Camera.h"

ParticleManager* ParticleManager::GetInstance()
{
	static ParticleManager instance;
	return &instance;
}

void ParticleManager::Initialize(DirectXCommon* dxCommon,SrvManager* srvManager)
{
	dxCommon_ = dxCommon;
	srvManager_ = srvManager;

	//ランダムエンジンの初期化
	//std::random_device seedGenerator;
	//std::mt19937 randomEngine_;
	randomEngine_.seed(seedGenerator());

	std::uniform_real_distribution<float>posDist(-1.0f, 1.0f);
	std::uniform_real_distribution<float>velDist(-1.0f, 1.0f);

	CreateRootSignature();
	CreateGraphicsPipelineState();

	CreateVertexBuffer();

	ID3D12Device* device = dxCommon_->GetDevice();
	D3D12_HEAP_PROPERTIES uploadHeapProperties{};
	uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;

	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resourceDesc.Width = sizeof(ParticleForGPU) * kNumInstances;
	resourceDesc.Height = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	HRESULT hr = device->CreateCommittedResource(
		&uploadHeapProperties, D3D12_HEAP_FLAG_NONE,
		&resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
		IID_PPV_ARGS(&instancingResource)
	);
	assert(SUCCEEDED(hr));

	//バッファに書き込むためのポインタを取得
	instancingResource->Map(0, nullptr, reinterpret_cast<void**>(&instancingData));

	base.transform.scale = { 1.0f,1.0f,1.0f };
	// 10個の初期位置を適当に横に並べておく
	for (uint32_t i = 0; i < kNumInstances; ++i)
	{
		particles_[i] = MakeNewParticle(randomEngine_);
	}
}

void ParticleManager::Update(Camera* camera)
{
	const float kDeltaTime = 1.0f / 60.0f;

	for (int i = 0; i < kNumInstances; ++i)
	{
		//ImGui用の
		Particle final;
		final.transform.scale = particles_[i].transform.scale * base.transform.scale;										  
		final.transform.rotate = particles_[i].transform.rotate + base.transform.rotate;
		
		particles_[i].transform.translate += particles_[i].velocity * kDeltaTime;
		
		//std::uniform_real_distribution<float>distribution(-1.0f, 1.0f);
		//particles_[i].transform = { distribution(randomEngine_),distribution(randomEngine_),distribution(randomEngine_) };
		//particles_[i].velocity = { distribution(randomEngine_),distribution(randomEngine_),distribution(randomEngine_) };
		
		final.transform.translate = particles_[i].transform.translate + base.transform.translate;
		
		Matrix4x4 worldMatrix = MatrixMath::MakeAffineMatrix(final.transform.scale, final.transform.rotate, final.transform.translate);
		
		instancingData[i].world = worldMatrix;
		instancingData[i].WVP = MatrixMath::Multiply(worldMatrix,camera->GetViewProjectionMatrix());
	}
}

void ParticleManager::Draw(D3D12_GPU_DESCRIPTOR_HANDLE srvHandleGPU)
{
	auto commandList = dxCommon_->GetCommandList();

	commandList->SetGraphicsRootSignature(rootSignature_.Get());
	commandList->SetPipelineState(graphicsPipelineState_[static_cast<int>(BlendMode::kBlendModeNormal)].Get());
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	commandList->IASetVertexBuffers(0, 1, &vertexBufferView_);

	commandList->SetGraphicsRootShaderResourceView(0, instancingResource->GetGPUVirtualAddress()); // ★ 0にする！
	commandList->SetGraphicsRootDescriptorTable(1, srvHandleGPU);

	commandList->DrawInstanced(6, kNumInstances, 0, 0);

}

void ParticleManager::Finalize()
{

}
	
void ParticleManager::CreateRootSignature()
{
	ID3D12Device* device = dxCommon_->GetDevice();

	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
	descriptorRange[0].BaseShaderRegister = 0; // t0
	descriptorRange[0].NumDescriptors = 1;
	descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	//RootParameter作成
	D3D12_ROOT_PARAMETER rootParamenters[4] = {};
	// [0] インスタンシング用 (SRV / t0 / 頂点シェーダー用)
	rootParamenters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
	rootParamenters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	rootParamenters[0].Descriptor.ShaderRegister = 0;
	rootParamenters[0].Descriptor.RegisterSpace = 0;

	// [1] テクスチャ用 (DescriptorTable / t0 / ピクセルシェーダー用)
	rootParamenters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParamenters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParamenters[1].DescriptorTable.pDescriptorRanges = descriptorRange;
	rootParamenters[1].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange);

	// [2] マテリアル用 (CBV / b0 / ピクセルシェーダー用)
	rootParamenters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParamenters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParamenters[2].Descriptor.ShaderRegister = 0; // ★ b0
	rootParamenters[2].Descriptor.RegisterSpace = 0;

	// [3] ライト用 (CBV / b1 / ピクセルシェーダー用)
	rootParamenters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParamenters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParamenters[3].Descriptor.ShaderRegister = 1; // ★ b1
	rootParamenters[3].Descriptor.RegisterSpace = 0;

	descriptionRootSignature.pParameters = rootParamenters;
	descriptionRootSignature.NumParameters = _countof(rootParamenters);


	// サンプラーの設定
	D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
	staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX;
	staticSamplers[0].ShaderRegister = 0; // s0
	staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	descriptionRootSignature.pStaticSamplers = staticSamplers;
	descriptionRootSignature.NumStaticSamplers = _countof(staticSamplers);

	ID3DBlob* signatureBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;

	HRESULT hr = D3D12SerializeRootSignature(&descriptionRootSignature,
		D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);

	assert(SUCCEEDED(hr));

	// GPU上にルートシグネチャを作成
	hr = device->CreateRootSignature(
		0,
		signatureBlob->GetBufferPointer(),
		signatureBlob->GetBufferSize(),
		IID_PPV_ARGS(&rootSignature_));

	assert(SUCCEEDED(hr));

	if (signatureBlob) { signatureBlob->Release(); }
	if (errorBlob) { errorBlob->Release(); }
}

void ParticleManager::CreateGraphicsPipelineState()
{
	ID3D12Device* device = dxCommon_->GetDevice();

	//Shaderを Particle 用に変更！
	Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob = dxCommon_->CompileShader(L"Particle.VS.hlsl", L"vs_6_0");
	assert(vertexShaderBlob != nullptr);

	Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlob = dxCommon_->CompileShader(L"Particle.PS.hlsl", L"ps_6_0");
	assert(pixelShaderBlob != nullptr);

	//インプットレイアウト
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[3] = {};
	inputElementDescs[0].SemanticName = "POSITION";
	inputElementDescs[0].SemanticIndex = 0;
	inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	inputElementDescs[1].SemanticName = "TEXCOORD";
	inputElementDescs[1].SemanticIndex = 0;
	inputElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	inputElementDescs[2].SemanticName = "NORMAL";
	inputElementDescs[2].SemanticIndex = 0;
	inputElementDescs[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputElementDescs[2].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	inputLayoutDesc.pInputElementDescs = inputElementDescs;
	inputLayoutDesc.NumElements = _countof(inputElementDescs);

	// RasterizerState
	D3D12_RASTERIZER_DESC rasterizerDesc{};
	rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE; // 裏面も描画
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

	// DepthStencilStateの設定（書き込みを ZERO にする！）
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO; // パーティクルはZバッファに書き込まない
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

	// PSOの共通設定
	D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineDesc{};
	pipelineDesc.pRootSignature = rootSignature_.Get();
	pipelineDesc.InputLayout = inputLayoutDesc;
	pipelineDesc.VS = { vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize() };
	pipelineDesc.PS = { pixelShaderBlob->GetBufferPointer(), pixelShaderBlob->GetBufferSize() };
	pipelineDesc.RasterizerState = rasterizerDesc;
	pipelineDesc.NumRenderTargets = 1;
	pipelineDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	pipelineDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	pipelineDesc.SampleDesc.Count = 1;
	pipelineDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	pipelineDesc.DepthStencilState = depthStencilDesc;
	pipelineDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

	// ここが一番大事！ブレンドモードの数だけループしてパイプラインを作る！
	for (int i = 0; i < static_cast<int>(BlendMode::kCountOfBlendMode); ++i)
	{
		BlendMode mode = static_cast<BlendMode>(i);

		// モードに応じたブレンド設定をセット
		pipelineDesc.BlendState = GetBlendDesc(mode);

		// パイプラインを配列に保存
		HRESULT hr = device->CreateGraphicsPipelineState(&pipelineDesc, IID_PPV_ARGS(&graphicsPipelineState_[i]));
		assert(SUCCEEDED(hr));
	}
}

void ParticleManager::CreateVertexBuffer()
{
	ID3D12Device* device = dxCommon_->GetDevice();

	UINT vertexCount = 6;
	UINT vertexBufferSize = sizeof(VertexData) * vertexCount;

	D3D12_HEAP_PROPERTIES uploadHeapProperties{};
	uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;

	D3D12_RESOURCE_DESC vertexResourceDesc{};
	vertexResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	vertexResourceDesc.Width = sizeof(VertexData) * 6;//リソースのサイズ。今回はVector4を3頂点
	vertexResourceDesc.Height = 1;
	vertexResourceDesc.DepthOrArraySize = 1;
	vertexResourceDesc.MipLevels = 1;
	vertexResourceDesc.SampleDesc.Count = 1;
	vertexResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	HRESULT hr = device->CreateCommittedResource(
		&uploadHeapProperties,D3D12_HEAP_FLAG_NONE,
		&vertexResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
		IID_PPV_ARGS(&vertexResource_)
	);
	assert(SUCCEEDED(hr));

	vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	vertexBufferView_.SizeInBytes = vertexBufferSize;
	vertexBufferView_.StrideInBytes = sizeof(VertexData);

	VertexData* vertexData = nullptr;
	vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));

	//１枚目の三角形
	vertexData[0].position = { -0.5f, -0.5f, 0.0f, 1.0f };
	vertexData[0].texcoord = { 0.0f,1.0f };
	vertexData[0].normal = { 0.0f,0.0f,-1.0f };

	vertexData[1].position = { -0.5f,  0.5f, 0.0f, 1.0f };
	vertexData[1].texcoord = { 0.0f,0.0f };
	vertexData[1].normal = { 0.0f,0.0f,-1.0f };

	vertexData[2].position = { 0.5f, -0.5f, 0.0f, 1.0f };
	vertexData[2].texcoord = { 1.0f,1.0f };
	vertexData[2].normal = { 0.0f,0.0f,-1.0f };

	//２枚目の三角形
	vertexData[3].position = { -0.5f,  0.5f, 0.0f, 1.0f };
	vertexData[3].texcoord = { 0.0f,0.0f };
	vertexData[3].normal = { 0.0f,0.0f,-1.0f };

	vertexData[4].position = { 0.5f,  0.5f, 0.0f, 1.0f };
	vertexData[4].texcoord = { 1.0f,0.0f };
	vertexData[4].normal = { 0.0f,0.0f,-1.0f };
	
	vertexData[5].position = { 0.5f, -0.5f, 0.0f, 1.0f };
	vertexData[5].texcoord = { 1.0f,1.0f };
	vertexData[5].normal = { 0.0f,0.0f,-1.0f };

	vertexResource_->Unmap(0, nullptr);

}

D3D12_BLEND_DESC ParticleManager::GetBlendDesc(BlendMode mode)
{
	D3D12_BLEND_DESC blendDesc{};
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	blendDesc.RenderTarget[0].BlendEnable = TRUE;

	//アルファチャンネル設定(共通)
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;

	switch (mode)
	{
	case BlendMode::kBlendModeNormal:   //通常
		blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
		blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
		break;
	case BlendMode::kBlendModeAdd:      //加算
		blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
		break;
	case BlendMode::kBlendModeSubtract: //減算
		blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
		blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_REV_SUBTRACT;
		blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
		break;
	case BlendMode::kBlendModeMultiply: //乗算
		blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_ZERO;
		blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_SRC_COLOR;
		break;
	case BlendMode::kBlendModeScreen:   //スクリーン
		blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_INV_DEST_COLOR;
		blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
		break;
	}
	return blendDesc;

}

void ParticleManager::DrawImGui()
{
	ImGui::Begin("Particle Manager");
	ImGui::DragFloat3("Position", &base.transform.translate.x, 0.01f);
	ImGui::DragFloat3("Rotation", &base.transform.rotate.x, 0.01f);
	ImGui::DragFloat3("Scale", &base.transform.scale.x, 0.01f);
	ImGui::End();
}

Particle ParticleManager::MakeNewParticle(std::mt19937& randomEngine)
{
	std::uniform_real_distribution<float>distribution(-1.0f, 1.0f);
	Particle particle;
	particle.transform.scale = { 1.0f,1.0f,1.0f };
	particle.transform.rotate = { 0.0f,0.0f,0.0f };
	particle.transform.translate = { distribution(randomEngine),distribution(randomEngine) ,distribution(randomEngine) };
	particle.velocity = { distribution(randomEngine) ,distribution(randomEngine) ,distribution(randomEngine) };
	return particle;
}