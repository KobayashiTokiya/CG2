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


	CreateRootSignature();
	CreateGraphicsPipelineState();

	CreateVertexBuffer();
	CreateCylinderVertexBuffer();

	instancingResource = dxCommon_->CreateBufferResource(sizeof(ParticleForGPU) * kNumMaxInstance);
	
	//バッファに書き込むためのポインタを取得
	instancingResource->Map(0, nullptr, reinterpret_cast<void**>(&instancingData));

	base.transform.scale = { 1.0f,1.0f,1.0f };
	// 10個の初期位置を適当に横に並べておく
	//for (uint32_t i = 0; i < kNumMaxInstance; ++i)
	//{
	//particles.push_back(MakeNewParticle(randomEngine_));
	//particles.push_back(MakeNewParticle(randomEngine_));
	//particles.push_back(MakeNewParticle(randomEngine_));
		//(*particleIterator) = MakeNewParticle(randomEngine_);
	//}

	//エミッタ初期化
	emitter.count = 2;
	emitter.frequency = 0.1f;//0.5秒ごとに発生
	emitter.frequencyTime = 0.0f;//発生頻度用の時刻、0で初期化
	emitter.transform.translate = { 0.0f,0.0f,0.0f };
	emitter.transform.rotate = { 0.0f,0.0f,0.0f };
	emitter.transform.scale = { 1.0f,1.0f,1.0f };

	//Field初期化
	accelerationField.acceleration = { 15.0f,0.0f,0.0f };
	accelerationField.area.min = { -1.0f,-1.0f,-1.0f };
	accelerationField.area.max = { 1.0f,1.0f,1.0f };
}

void ParticleManager::Update(Camera* camera)
{
	const float kDeltaTime = 1.0f / 60.0f;
	numInstance = 0; // ループの前にインスタンス数をリセット

	if (!particles.empty()&&particles.front().type!=currentSpawnType)
	{
		particles.clear();
	}

	//emitter.frequencyTime += kDeltaTime;
	//if (emitter.frequencyTime >= emitter.frequency)
	//{
	//	// 時間が経ったら自動でパーティクルをリストに追加
	//	particles.splice(particles.end(), Emit(emitter, raandomEngine_));
	//	emitter.frequencyTime -= emitter.frequency; // タイマーを戻す
	//}

	// 発生している全パーティクルの更新ループ
	for (auto it = particles.begin(); it != particles.end(); )
	{
		// 1. 寿命のカウントと削除チェック
		it->currentTime += kDeltaTime;
		if (it->currentTime >= it->lifeTime)
		{
			it = particles.erase(it); // 寿命が尽きたらリストから削除して次の要素へ
			continue;
		}

		float expansionSpeed = 5.0f;
		if (it->type==ParticleType::kRing)
		{
			it->transform.scale.x += expansionSpeed * kDeltaTime;
			it->transform.scale.z += expansionSpeed * kDeltaTime;
		}
		else
		{
			it->transform.scale.x += expansionSpeed * kDeltaTime;
			it->transform.scale.z += expansionSpeed * kDeltaTime;
			it->transform.scale.y = 2.0f;
		}

		Matrix4x4 rotateMatrix;
		if (it->type==ParticleType::kRing)
		{
			Matrix4x4 rotateXMatrix = MatrixMath::MakeRotateXMatrix(0.0f);
			Matrix4x4 rotateYMatrix = MatrixMath::MakeRotateYMatrix(it->transform.rotate.y);
			Matrix4x4 rotateZMatrix = MatrixMath::MakeRotateZMatrix(it->transform.rotate.z);
		
			rotateMatrix = MatrixMath::Multiply(rotateXMatrix, rotateYMatrix);
			rotateMatrix = MatrixMath::Multiply(rotateMatrix, rotateZMatrix);
		}
		else
		{
			Matrix4x4 rotateXMatrix = MatrixMath::MakeRotateXMatrix(0.0f);
			Matrix4x4 rotateYMatrix = MatrixMath::MakeRotateYMatrix(it->transform.rotate.y);
			Matrix4x4 rotateZMatrix = MatrixMath::MakeRotateZMatrix(it->transform.rotate.z);

			rotateMatrix = MatrixMath::Multiply(rotateXMatrix, rotateYMatrix);
			rotateMatrix = MatrixMath::Multiply(rotateMatrix, rotateZMatrix);
		}

		Matrix4x4 scaleMatrix = MatrixMath::MakeScaleMatrix(it->transform.scale);
		Matrix4x4 translateMatrix = MatrixMath::MakeTranslateMatrix(it->transform.translate);

		Matrix4x4 worldMatrix = MatrixMath::Multiply(scaleMatrix, rotateMatrix);
		worldMatrix = MatrixMath::Multiply(worldMatrix, translateMatrix);

		// 5. 最大数を超えないようにグラフィックメモリ（インスタンス用バッファ）に転送
		if (numInstance < kNumMaxInstance)
		{
			instancingData[numInstance].world = worldMatrix;

			// WVPの計算
			Matrix4x4 worldView = MatrixMath::Multiply(worldMatrix, camera->GetViewMatrix());
			instancingData[numInstance].WVP = MatrixMath::Multiply(worldView, camera->GetProjectionMatrix());

			// 生存時間（currentTime / lifeTime）に応じてアルファ値を下げ、フワッと消えさせる
			float alpha = 1.0f - (it->currentTime / it->lifeTime);
			instancingData[numInstance].color = { it->color.x, it->color.y, it->color.z, alpha };

			numInstance++; // 描画するインスタンス数をカウントアップ
		}

		++it;
	}
}

void ParticleManager::Draw(D3D12_GPU_DESCRIPTOR_HANDLE srvHandleGPU)
{
	// 描画するパーティクルが画面に1つもない時は処理をスキップ
	if (numInstance == 0) return;

	auto commandList = dxCommon_->GetCommandList();

	commandList->SetGraphicsRootSignature(rootSignature_.Get());
	commandList->SetPipelineState(graphicsPipelineState_[static_cast<int>(BlendMode::kBlendModeAdd)].Get());
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	commandList->SetGraphicsRootShaderResourceView(0, instancingResource->GetGPUVirtualAddress());
	commandList->SetGraphicsRootDescriptorTable(1, srvHandleGPU);

	if (currentSpawnType==ParticleType::kRing)
	{
		commandList->IASetVertexBuffers(0, 1, &vertexBufferView_);
		commandList->DrawInstanced((UINT)vertices_.size(), numInstance, 0, 0);
	}
	else
	{
		commandList->IASetVertexBuffers(0, 1, &cylinderVertexBufferView_);
		commandList->DrawInstanced((UINT)cylinderVertices_.size(), numInstance, 0, 0);
	}

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
	staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
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
	rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE; // 裏面も描画(D3D12_CULL_MODE_NONE)　裏面を表示しない(D3D12_CULL_MODE_BACK)　表を表示しない(D3D12_CULL_MODE_FRONT)
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

	// ★スライド4枚目の仕様通りに設定
	const uint32_t kRingDivide = 32;
	const float kOuterRadius = 1.0f;
	const float kInnerRadius = 0.2f;
	const float radianPerDivide = 2.0f * std::numbers::pi_v<float> / float(kRingDivide);

	// 1つの分割につき2つの三角形（6頂点）を作るので、合計頂点数を計算
	vertices_.resize(kRingDivide * 6);

	UINT vertexCount = (UINT)vertices_.size();
	UINT vertexBufferSize = sizeof(VertexData) * vertexCount;

	// ヒープとリソースの作成（既存のコードの通り）
	D3D12_HEAP_PROPERTIES uploadHeapProperties{};
	uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;

	D3D12_RESOURCE_DESC vertexResourceDesc{};
	vertexResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	vertexResourceDesc.Width = vertexBufferSize;
	vertexResourceDesc.Height = 1;
	vertexResourceDesc.DepthOrArraySize = 1;
	vertexResourceDesc.MipLevels = 1;
	vertexResourceDesc.SampleDesc.Count = 1;
	vertexResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	HRESULT hr = device->CreateCommittedResource(
		&uploadHeapProperties, D3D12_HEAP_FLAG_NONE,
		&vertexResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
		IID_PPV_ARGS(&vertexResource_)
	);
	assert(SUCCEEDED(hr));

	vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	vertexBufferView_.SizeInBytes = vertexBufferSize;
	vertexBufferView_.StrideInBytes = sizeof(VertexData);

	VertexData* vertexData = nullptr;
	vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));

	// ★スライドのアルゴリズムに従って頂点データをループ生成
	size_t vIdx = 0;
	for (uint32_t index = 0; index < kRingDivide; ++index)
	{
		float sin = std::sin(index * radianPerDivide);
		float cos = std::cos(index * radianPerDivide);
		float sinNext = std::sin((index + 1) * radianPerDivide);
		float cosNext = std::cos((index + 1) * radianPerDivide);

		// 円周方向は固定（0.5f）、内側・外側の半径方向で 0.0f 〜 1.0f のグラデーションを作ります
		// ※もしこれで見た目が変わらない、あるいはまだトゲトゲする場合は、
		// 下記の {0.5f, 0.0f} と {0.5f, 1.0f} の中身を {0.0f, 0.5f} と {1.0f, 0.5f} に入れ替えてみてください。

		// 三角形1枚目 (① -> ② -> ③)
		vertexData[vIdx++] = { {cos * kOuterRadius, 0.0f, sin * kOuterRadius, 1.0f}, {0.5f, 0.0f}, {0.0f, 1.0f, 0.0f} }; // ① 外側1
		vertexData[vIdx++] = { {cos * kInnerRadius, 0.0f, sin * kInnerRadius, 1.0f}, {0.5f, 1.0f}, {0.0f, 1.0f, 0.0f} }; // ③ 内側1
		vertexData[vIdx++] = { {cosNext * kOuterRadius, 0.0f, sinNext * kOuterRadius, 1.0f}, {0.5f, 0.0f}, {0.0f, 1.0f, 0.0f} }; // ② 外側2

		// 三角形2枚目 (② -> ④ -> ③)
		vertexData[vIdx++] = { {cosNext * kOuterRadius, 0.0f, sinNext * kOuterRadius, 1.0f}, {0.5f, 0.0f}, {0.0f, 1.0f, 0.0f} }; // ② 外側2
		vertexData[vIdx++] = { {cos * kInnerRadius, 0.0f, sin * kInnerRadius, 1.0f}, {0.5f, 1.0f}, {0.0f, 1.0f, 0.0f} }; // ③ 内側1
		vertexData[vIdx++] = { {cosNext * kInnerRadius, 0.0f, sinNext * kInnerRadius, 1.0f}, {0.5f, 1.0f}, {0.0f, 1.0f, 0.0f} }; // ④ 内側2
	}

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
	
	ImGui::DragFloat3("EmitterTranslate", &emitter.transform.translate.x, 0.01f, -100.0f, 100.0f);
	if (ImGui::Button("Add Particle"))
	{
		particles.splice(particles.end(), Emit(emitter, randomEngine_));
	}

	if (ImGui::RadioButton("Ring", currentSpawnType == ParticleType::kRing))
	{
		currentSpawnType = ParticleType::kRing;
	}
	ImGui::SameLine();
	if (ImGui::RadioButton("Cylinder", currentSpawnType == ParticleType::kCylinder))
	{
		currentSpawnType = ParticleType::kCylinder;
	}
	ImGui::End();
}

Particle ParticleManager::MakeNewParticle(std::mt19937& randomEngine, const Vector3& translate)
{
	Particle particle;
	particle.type = currentSpawnType;

	if (particle.type==ParticleType::kRing)
	{
		particle.transform.scale = { 3.0f,3.0f,3.0f };
	}
	else
	{
		particle.transform.scale = { 2.0f,3.0f,2.0f };
	}

	particle.transform.rotate = { 0.0f,0.0f,0.0f };
	particle.transform.translate = translate;
	particle.velocity = { 0.0f,0.0f,0.0f };
	particle.color = { 1.0f, 1.0f, 1.0f, 1.0f };

	std::uniform_real_distribution<float> distTime(1.0f, 2.0f);
	particle.lifeTime = distTime(randomEngine);
	particle.currentTime = 0.0f;

	return particle;
}

std::list<Particle> ParticleManager::Emit(const Emitter& emitter, std::mt19937& randomEngine)
{
	std::list<Particle>particles;
	for (uint32_t count = 0; count < emitter.count; ++count)
	{
		particles.push_back(MakeNewParticle(randomEngine,emitter.transform.translate));
	}
	return particles;
}

void ParticleManager::CreateCylinderVertexBuffer()
{
	ID3D12Device* device = dxCommon_->GetDevice();

	const uint32_t kCylinderDivide = 32;
	const float kTopRadius = 1.0f;
	const float kBottomRadius = 1.0f;
	const float kHeight = 3.0f;
	const float radianperDivide = 2.0f*std::numbers::pi_v<float>/float(kCylinderDivide);

	cylinderVertices_.resize(kCylinderDivide * 6);
	UINT vertexCount = (UINT)cylinderVertices_.size();
	UINT vertexBufferSize = sizeof(VertexData) * vertexCount;

	D3D12_HEAP_PROPERTIES uploadHeapProperties{ .Type = D3D12_HEAP_TYPE_UPLOAD };
	D3D12_RESOURCE_DESC  vertexResourceDesc{
		.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER,
		.Width = vertexBufferSize,
		.Height = 1,.DepthOrArraySize = 1,.MipLevels = 1,
		.SampleDesc = {.Count = 1},.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR
	};

	HRESULT hr = device->CreateCommittedResource(
		&uploadHeapProperties, D3D12_HEAP_FLAG_NONE,
		&vertexResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
		IID_PPV_ARGS(&cylinderVertexResource_)
	);
	assert(SUCCEEDED(hr));

	cylinderVertexBufferView_.BufferLocation = cylinderVertexResource_->GetGPUVirtualAddress();
	cylinderVertexBufferView_.SizeInBytes = vertexBufferSize;
	cylinderVertexBufferView_.StrideInBytes = sizeof(VertexData);

	VertexData* vertexData = nullptr;
	cylinderVertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));

	size_t vIdx = 0;
	for (uint32_t index = 0; index < kCylinderDivide; index++)
	{
		float sin = std::sin(index * radianperDivide);
		float cos = std::cos(index * radianperDivide);
		float sinNext = std::sin((index+1)* radianperDivide);
		float cosNext = std::cos((index + 1) * radianperDivide);
		float u = float(index) / float(kCylinderDivide);
		float uNext = float(index + 1) / float(kCylinderDivide);

		vertexData[vIdx++] = { {-sin * kTopRadius,kHeight,cos * kTopRadius,1.0f},{u,0.0f},{-sin,0.0f,cos} };
		vertexData[vIdx++] = { {-sinNext * kTopRadius,kHeight,cosNext * kTopRadius,1.0f},{uNext,0.0f},{-sinNext,0.0f,cosNext} };
		vertexData[vIdx++] = { {-sin * kBottomRadius,0.0f,cos * kBottomRadius,1.0f},{u,1.0f},{-sin,0.0f,cos} };
		vertexData[vIdx++] = { {-sin * kBottomRadius,0.0f,cos * kBottomRadius,1.0f},{u,1.0f},{-sin,0.0f,cos} };
		vertexData[vIdx++] = { {-sinNext * kTopRadius,kHeight,cosNext * kTopRadius,1.0f},{uNext,0.0f},{-sinNext,0.0f,cosNext} };
		vertexData[vIdx++] = { {-sinNext * kBottomRadius,0.0f,cosNext * kBottomRadius,1.0f},{uNext,1.0f},{-sinNext,0.0f,cosNext} };
	}
	cylinderVertexResource_->Unmap(0, nullptr);

}