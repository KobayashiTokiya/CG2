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
	CreateSphereVertexBuffer();
	CreateLightningVertexBuffer();

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

	//emitter.frequencyTime += kDeltaTime;
	//if (emitter.frequencyTime >= emitter.frequency)
	//{
	//	particles.splice(particles.end(), Emit(emitter, randomEngine_));
	//	emitter.frequencyTime -= emitter.frequency;
	//}
	if (isTriggeredInazumaBreak)
	{
		inazumaBreakTimer += kDeltaTime;

		// 球体が最大に成長して消えるまでの間（1秒間）継続して稲妻を出す
		if (inazumaBreakTimer < 1.0f)
		{
			static float lightningInterval = 0.0f;
			lightningInterval += kDeltaTime;

			// 0.05秒（約3フレーム）ごとにバチバチッと継続生成
			if (lightningInterval >= 0.05f)
			{
				ParticleType backupType = currentSpawnType;
				currentSpawnType = ParticleType::kLightning;

				Emitter continuousLightning = emitter;
				continuousLightning.transform.translate = inazumaBreakPosition;
				continuousLightning.count = 2; // 1回につき2本追加
				particles.splice(particles.end(), Emit(continuousLightning, randomEngine_));

				currentSpawnType = backupType;
				lightningInterval = 0.0f;
			}
		}
		else
		{
			// 1.0秒経ったらチャージ演出終了
			isTriggeredInazumaBreak = false;
		}
	}

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
		if (it->type == ParticleType::kRing)
		{
			it->transform.scale.x += expansionSpeed * kDeltaTime;
			it->transform.scale.z += expansionSpeed * kDeltaTime;
		}
		else if (it->type == ParticleType::kSphere)
		{
			if (it->transform.scale.x < 2.0f &&
				it->transform.scale.y < 2.0f &&
				it->transform.scale.z < 2.0f)
			{
				it->transform.scale.x += expansionSpeed * kDeltaTime;
				it->transform.scale.y += expansionSpeed * kDeltaTime;
				it->transform.scale.z += expansionSpeed * kDeltaTime;
			}
			else
			{
				it->transform.scale.x = 2.0f;
				it->transform.scale.y = 2.0f;
				it->transform.scale.z = 2.0f;
			}
			currentSphereScale = it->transform.scale.x;
		}
		else if (it->type == ParticleType::kLightning)
		{
			it->transform.translate.x += it->velocity.x * kDeltaTime;
			it->transform.translate.y += it->velocity.y * kDeltaTime;
			it->transform.translate.z += it->velocity.z * kDeltaTime;
		}
		else
		{
			it->transform.scale.x += expansionSpeed * kDeltaTime;
			it->transform.scale.z += expansionSpeed * kDeltaTime;
			it->transform.scale.y = 2.0f;
		}
		++it;
	}
}
void ParticleManager::Draw(
	Camera* camera,
	D3D12_GPU_DESCRIPTOR_HANDLE srvHandleGPU, // Ring用テクスチャ
	D3D12_GPU_DESCRIPTOR_HANDLE cylinderSrvHandle,
	D3D12_GPU_DESCRIPTOR_HANDLE sphereSrvHandle,
	D3D12_GPU_DESCRIPTOR_HANDLE lightningSrvHandle)
{
	// 描画するパーティクルが画面に1つもない時は処理をスキップ
	if (particles.empty()) return;

	auto commandList = dxCommon_->GetCommandList();

	commandList->SetGraphicsRootSignature(rootSignature_.Get());
	commandList->SetPipelineState(graphicsPipelineState_[static_cast<int>(BlendMode::kBlendModeNormal)].Get());
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	commandList->SetGraphicsRootShaderResourceView(0, instancingResource->GetGPUVirtualAddress());

	// 全種類で共有する、インスタンスバッファ内の絶対的な書き込み位置
	uint32_t globalInstanceCount = 0;

	// 各タイプごとに仕分けしてバッファに詰めるラムダ関数
	auto DrawType = [&](ParticleType targetType, D3D12_GPU_DESCRIPTOR_HANDLE srvHandle, D3D12_VERTEX_BUFFER_VIEW* vbView, size_t vertexCount)
		{
			uint32_t batchNumInstance = 0;              // この種類単体での描画数
			uint32_t startInstanceLocation = globalInstanceCount; // この種類のバッファ開始位置

			// リストの中から「今から描画したいタイプ」だけを抽出してインスタンスバッファに詰める
			for (const auto& part : particles)
			{
				// 対象のタイプではないパーティクルは、カウントを進めずに完全に無視する！
				if (part.type != targetType) continue;
				if (globalInstanceCount >= kNumMaxInstance) break; // 全体の最大数を超えたら終了

				Matrix4x4 rotateMatrix;
				if (part.type == ParticleType::kRing)
				{
					Matrix4x4 rotateXMatrix = MatrixMath::MakeRotateXMatrix(0.0f);
					Matrix4x4 rotateYMatrix = MatrixMath::MakeRotateYMatrix(part.transform.rotate.y);
					Matrix4x4 rotateZMatrix = MatrixMath::MakeRotateZMatrix(part.transform.rotate.z);
					rotateMatrix = MatrixMath::Multiply(rotateXMatrix, rotateYMatrix);
					rotateMatrix = MatrixMath::Multiply(rotateMatrix, rotateZMatrix);
				}
				else if (part.type == ParticleType::kLightning)
				{
					Matrix4x4 rotateXMatrix = MatrixMath::MakeRotateXMatrix(0.0f);
					Matrix4x4 rotateYMatrix = MatrixMath::MakeRotateYMatrix(0.0f);
					Matrix4x4 rotateZMatrix = MatrixMath::MakeRotateZMatrix(part.transform.rotate.z);
					rotateMatrix = MatrixMath::Multiply(rotateXMatrix, rotateYMatrix);
					rotateMatrix = MatrixMath::Multiply(rotateMatrix, rotateZMatrix);
				}
				else
				{
					Matrix4x4 rotateXMatrix = MatrixMath::MakeRotateXMatrix(0.0f);
					Matrix4x4 rotateYMatrix = MatrixMath::MakeRotateYMatrix(part.transform.rotate.y);
					Matrix4x4 rotateZMatrix = MatrixMath::MakeRotateZMatrix(part.transform.rotate.z);
					rotateMatrix = MatrixMath::Multiply(rotateXMatrix, rotateYMatrix);
					rotateMatrix = MatrixMath::Multiply(rotateMatrix, rotateZMatrix);
				}

				// 稲妻（kLightning）のとき、元の芯が極小なのでもっと大胆に太く・長くする
				Vector3 finalScale = part.transform.scale;
				if (part.type == ParticleType::kLightning)
				{
					finalScale.x = part.transform.scale.x * 250.0f; // 横幅をさらに太く
					finalScale.y = part.transform.scale.y * 300.0f; // 縦（長さ）をさらに長く
					finalScale.z = part.transform.scale.z * 250.0f;
				}

				// 補正した finalScale を使ってスケール行列を作ります
				Matrix4x4 scaleMatrix = MatrixMath::MakeScaleMatrix(finalScale);
				Matrix4x4 translateMatrix = MatrixMath::MakeTranslateMatrix(part.transform.translate);

				Matrix4x4 worldMatrix = MatrixMath::Multiply(scaleMatrix, rotateMatrix);
				worldMatrix = MatrixMath::Multiply(worldMatrix, translateMatrix);
				// ------------------------------------------

				// 全体で一意のインデックス（globalInstanceCount）の位置にデータを転送
				instancingData[globalInstanceCount].world = worldMatrix;

				// WVPの計算
				Matrix4x4 worldView = MatrixMath::Multiply(worldMatrix, camera->GetViewMatrix());
				instancingData[globalInstanceCount].WVP = MatrixMath::Multiply(worldView, camera->GetProjectionMatrix());

				// アルファ値の計算
				float alpha = 1.0f - (part.currentTime / part.lifeTime);
				if (alpha < 0.0f) alpha = 0.0f;
				instancingData[globalInstanceCount].color = { part.color.x, part.color.y, part.color.z, alpha };

				globalInstanceCount++; // 正しい対象のときだけ書き込み位置を進める
				batchNumInstance++;    // このバッチの数を増やす
			}

			// 対象のインスタンスが1つ以上あれば、対応するテクスチャと頂点バッファを設定して描画
			if (batchNumInstance > 0)
			{
				commandList->SetGraphicsRootDescriptorTable(1, srvHandle);
				commandList->IASetVertexBuffers(0, 1, vbView);

				// 第4引数（StartInstanceLocation）に startInstanceLocation を渡す
				commandList->DrawInstanced((UINT)vertexCount, batchNumInstance, 0, startInstanceLocation);
			}
		};

	// --- 各タイプごとに仕分けして描画コマンドを発行 ---
	DrawType(ParticleType::kRing, srvHandleGPU, &vertexBufferView_, vertices_.size());
	DrawType(ParticleType::kCylinder, cylinderSrvHandle, &cylinderVertexBufferView_, cylinderVertices_.size());
	DrawType(ParticleType::kSphere, sphereSrvHandle, &sphereVertexBufferView_, sphereVertices_.size());

	// 稲妻（Lightning）を描画する直前で、加算ブレンド（Add）に切り替える！
	commandList->SetPipelineState(graphicsPipelineState_[static_cast<int>(BlendMode::kBlendModeAdd)].Get());
	DrawType(ParticleType::kLightning, lightningSrvHandle, &lightningVertexBufferView_, lightningVertices_.size());
}

std::list<Particle> ParticleManager::Emit(const Emitter& emitter, std::mt19937& randomEngine)
{
	std::list<Particle>particles;
	for (uint32_t count = 0; count < emitter.count; ++count)
	{
		particles.push_back(MakeNewParticle(randomEngine, emitter.transform.translate));
	}
	return particles;
}

void ParticleManager::EmitComboExplosion(const Vector3& position)
{
	// すでに演出（チャージ中）の場合は重ねて処理しない
	if (isTriggeredInazumaBreak) return;

	// タイマースイッチを起動
	isTriggeredInazumaBreak = true;
	inazumaBreakTimer = 0.0f;
	inazumaBreakPosition = position;

	// 起点となる球体を1個だけ生成
	ParticleType backupType = currentSpawnType;
	currentSpawnType = ParticleType::kSphere;

	Emitter comboEmitter = emitter;
	comboEmitter.transform.translate = position;
	comboEmitter.count = 1;
	particles.splice(particles.end(), Emit(comboEmitter, randomEngine_));

	currentSpawnType = backupType;
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

	if (ImGui::Button("Emit Combo Explosion!!"))
	{
		// エミッタの現在位置にコンボエフェクトを発生させる
		EmitComboExplosion(emitter.transform.translate);
	}
	ImGui::End();
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
	Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob = dxCommon_->CompileShader(L"Resource/shaders/Particle.VS.hlsl", L"vs_6_0");
	assert(vertexShaderBlob != nullptr);

	Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlob = dxCommon_->CompileShader(L"Resource/shaders/Particle.PS.hlsl", L"ps_6_0");
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

		// 加算ブレンド（kBlendModeAdd）の時だけ、深度テストをオフにして常に最前面に描画する
		if (mode == BlendMode::kBlendModeAdd)
		{
			pipelineDesc.DepthStencilState.DepthEnable = false;
		}
		else
		{
			pipelineDesc.DepthStencilState.DepthEnable = true; // 通常ブレンドなどは元の設定のまま
		}

		// パイプラインを配列に保存
		HRESULT hr = device->CreateGraphicsPipelineState(&pipelineDesc, IID_PPV_ARGS(&graphicsPipelineState_[i]));
		assert(SUCCEEDED(hr));
	}
}

void ParticleManager::CreateVertexBuffer()
{
	ID3D12Device* device = dxCommon_->GetDevice();

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

	size_t vIdx = 0;
	for (uint32_t index = 0; index < kRingDivide; ++index)
	{
		float sin = std::sin(index * radianPerDivide);
		float cos = std::cos(index * radianPerDivide);
		float sinNext = std::sin((index + 1) * radianPerDivide);
		float cosNext = std::cos((index + 1) * radianPerDivide);


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


Particle ParticleManager::MakeNewParticle(std::mt19937& randomEngine, const Vector3& translate)
{
	Particle particle;
	particle.type = currentSpawnType;

	// --- 共通の初期値（まずベースを設定する） ---
	particle.velocity = { 0.0f, 0.0f, 0.0f };
	particle.color = { 1.0f, 1.0f, 1.0f, 1.0f };
	particle.currentTime = 0.0f;

	// --- タイプ別の固有設定 ---
	if (particle.type == ParticleType::kRing)
	{
		particle.transform.scale = { 3.0f, 3.0f, 3.0f };
		particle.transform.rotate = { 0.0f, 0.0f, 0.0f };
		particle.transform.translate = translate;

		std::uniform_real_distribution<float> distTime(1.0f, 2.0f);
		particle.lifeTime = distTime(randomEngine);
	}
	else if (particle.type == ParticleType::kSphere)
	{
		particle.transform.scale = { 2.0f, 2.0f, 2.0f };
		particle.transform.rotate = { 0.0f, 0.0f, 0.0f };
		particle.transform.translate = translate;

		std::uniform_real_distribution<float> distTime(1.0f, 2.0f);
		particle.lifeTime = distTime(randomEngine);
	}
	else if (particle.type == ParticleType::kLightning)
	{
		//縦の基本長さを少し長めに（2.0倍の補正）
		particle.transform.scale = { 0.2f, 3.0f * (currentSphereScale / 2.0f) * 2.0f, 0.2f };

		std::uniform_real_distribution<float> distPos(-1.0f, 1.0f);
		Vector3 randomDir = { distPos(randomEngine), distPos(randomEngine), distPos(randomEngine) };

		// 方向ベクトルを正規化して綺麗な放射状にする
		float length = std::sqrt(randomDir.x * randomDir.x + randomDir.y * randomDir.y + randomDir.z * randomDir.z);
		if (length > 0.0f)
		{
			randomDir.x /= length;
			randomDir.y /= length;
			randomDir.z /= length;
		}

		// 球体の半径よりもさらに外側（1.5倍〜2.0倍の距離）から発生させて大きく見せる
		std::uniform_real_distribution<float> distRadius(1.5f, 2.0f);
		float radius = currentSphereScale * 0.5f * distRadius(randomEngine);

		// 初期位置を大きく散らす
		particle.transform.translate = {
			translate.x + randomDir.x * radius,
			translate.y + randomDir.y * radius,
			translate.z + randomDir.z * radius
		};

		//外側へ向かってバチバチと高速に飛び出させる
		particle.velocity = {
			randomDir.x * 15.0f,
			randomDir.y * 15.0f,
			randomDir.z * 15.0f
		};

		// XとYの回転は0に固定、Zだけランダム
		std::uniform_real_distribution<float> distRot(0.0f, 2.0f * std::numbers::pi_v<float>);
		particle.transform.rotate = { 0.0f, 0.0f, distRot(randomEngine) };

		std::uniform_real_distribution<float> distTime(0.05f, 0.12f);
		particle.lifeTime = distTime(randomEngine);

		std::uniform_int_distribution<int> distColorType(0, 1); // 0か1をランダムに決定
		if (distColorType(randomEngine) == 0)
		{
			particle.color = { 0.5f, 0.0f, 1.0f, 1.0f };
		}
		else
		{
			particle.color = { 1.0f, 0.9f, 0.1f, 1.0f };
		}
	}
	else // Cylinderなど
	{
		particle.transform.scale = { 2.0f, 3.0f, 2.0f };
		particle.transform.rotate = { 0.0f, 0.0f, 0.0f };
		particle.transform.translate = translate;

		std::uniform_real_distribution<float> distTime(1.0f, 2.0f);
		particle.lifeTime = distTime(randomEngine);
	}

	return particle;
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

void ParticleManager::CreateSphereVertexBuffer()
{
	ID3D12Device* device = dxCommon_->GetDevice();

	const uint32_t kSubdivision = 16; // 分割数（経度・緯度それぞれ）
	const float kRadius = 1.0f;       // 球体の半径

	// 緯度・経度の分割によって作成される全三角形の頂点数を計算してリサイズ
	// 1マスの四角形（三角形2枚=6頂点）が kSubdivision * kSubdivision 個並ぶ
	sphereVertices_.resize(kSubdivision * kSubdivision * 6);
	UINT vertexCount = (UINT)sphereVertices_.size();
	UINT vertexBufferSize = sizeof(VertexData) * vertexCount;

	D3D12_HEAP_PROPERTIES uploadHeapProperties{ .Type = D3D12_HEAP_TYPE_UPLOAD };
	D3D12_RESOURCE_DESC vertexResourceDesc{
		.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER,
		.Width = vertexBufferSize,
		.Height = 1, .DepthOrArraySize = 1, .MipLevels = 1,
		.SampleDesc = {.Count = 1}, .Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR
	};

	HRESULT hr = device->CreateCommittedResource(
		&uploadHeapProperties, D3D12_HEAP_FLAG_NONE,
		&vertexResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
		IID_PPV_ARGS(&sphereVertexResource_)
	);
	assert(SUCCEEDED(hr));

	sphereVertexBufferView_.BufferLocation = sphereVertexResource_->GetGPUVirtualAddress();
	sphereVertexBufferView_.SizeInBytes = vertexBufferSize;
	sphereVertexBufferView_.StrideInBytes = sizeof(VertexData);

	VertexData* vertexData = nullptr;
	sphereVertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));

	size_t vIdx = 0;
	float pi = std::numbers::pi_v<float>;

	// 緯度(phi)と経度(theta)のループで球体表面の頂点を生成
	for (uint32_t lat = 0; lat < kSubdivision; ++lat)
	{
		float phi = pi * float(lat) / float(kSubdivision);
		float phiNext = pi * float(lat + 1) / float(kSubdivision);

		for (uint32_t lon = 0; lon < kSubdivision; ++lon)
		{
			float theta = 2.0f * pi * float(lon) / float(kSubdivision);
			float thetaNext = 2.0f * pi * float(lon + 1) / float(kSubdivision);

			// 4つの頂点座標を計算
			auto GetSpherePos = [&](float p, float t) -> Vector4
				{
					return {
						kRadius * std::sin(p) * std::cos(t),
						kRadius * std::cos(p),
						kRadius * std::sin(p) * std::sin(t),
						1.0f
					};
				};

			Vector4 p0 = GetSpherePos(phi, theta);
			Vector4 p1 = GetSpherePos(phi, thetaNext);
			Vector4 p2 = GetSpherePos(phiNext, theta);
			Vector4 p3 = GetSpherePos(phiNext, thetaNext);

			// UV座標
			Vector2 uv0 = { float(lon) / kSubdivision, float(lat) / kSubdivision };
			Vector2 uv1 = { float(lon + 1) / kSubdivision, float(lat) / kSubdivision };
			Vector2 uv2 = { float(lon) / kSubdivision, float(lat + 1) / kSubdivision };
			Vector2 uv3 = { float(lon + 1) / kSubdivision, float(lat + 1) / kSubdivision };

			// 法線（球体なので中心からの向き＝座標のXYZと同じ向き）
			Vector3 n0 = { p0.x, p0.y, p0.z };
			Vector3 n1 = { p1.x, p1.y, p1.z };
			Vector3 n2 = { p2.x, p2.y, p2.z };
			Vector3 n3 = { p3.x, p3.y, p3.z };

			// 三角形1枚目 (p0 -> p1 -> p2)
			vertexData[vIdx++] = { p0, uv0, n0 };
			vertexData[vIdx++] = { p1, uv1, n1 };
			vertexData[vIdx++] = { p2, uv2, n2 };

			// 三角形2枚目 (p1 -> p3 -> p2)
			vertexData[vIdx++] = { p1, uv1, n1 };
			vertexData[vIdx++] = { p3, uv3, n3 };
			vertexData[vIdx++] = { p2, uv2, n2 };
		}
	}

	sphereVertexResource_->Unmap(0, nullptr);
}

void ParticleManager::CreateLightningVertexBuffer()
{
	ID3D12Device* device = dxCommon_->GetDevice();

	const uint32_t kSegments = 8;

	lightningVertices_.resize(kSegments * 6);
	UINT vertexCount = (UINT)lightningVertices_.size();
	UINT vertexBufferSize = sizeof(VertexData) * vertexCount;

	D3D12_HEAP_PROPERTIES uploadHeapProperties{ .Type = D3D12_HEAP_TYPE_UPLOAD };
	D3D12_RESOURCE_DESC vertexResourceDesc{
		.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER,
		.Width = vertexBufferSize,
		.Height = 1, .DepthOrArraySize = 1, .MipLevels = 1,
		.SampleDesc = {.Count = 1}, .Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR
	};

	HRESULT hr = device->CreateCommittedResource(
		&uploadHeapProperties, D3D12_HEAP_FLAG_NONE,
		&vertexResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
		IID_PPV_ARGS(&lightningVertexResource_)
	);
	assert(SUCCEEDED(hr));

	lightningVertexBufferView_.BufferLocation = lightningVertexResource_->GetGPUVirtualAddress();
	lightningVertexBufferView_.SizeInBytes = vertexBufferSize;
	lightningVertexBufferView_.StrideInBytes = sizeof(VertexData);

	VertexData* vertexData = nullptr;
	lightningVertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));

	float segmentHeight = 1.0f / float(kSegments);
	size_t vIdx = 0;

	std::vector<float> xOffsets(kSegments + 1, 0.0f);

	std::mt19937 localEngine(1337); 
	std::uniform_real_distribution<float> distOffset(-0.02f, 0.02f); // 左右のズレ幅

	// 根元（0）と先端（kSegments）は少し抑えめに、途中をバチバチにズラす
	for (uint32_t i = 1; i < kSegments; ++i)
	{
		xOffsets[i] = distOffset(localEngine);
	}

	for (uint32_t i = 0; i < kSegments; ++i)
	{
		// 縦の基本位置（100分の1スケール）
		float yBottom = (-0.5f + float(i) * segmentHeight) * 0.01f;
		float yTop = (-0.5f + float(i + 1) * segmentHeight) * 0.01f;

		float vBottom = 1.0f - float(i) / float(kSegments);
		float vTop = 1.0f - float(i + 1) / float(kSegments);

		//計算しておいたXのオフセット（ギザギザ）を適用する
		float xB = xOffsets[i];
		float xT = xOffsets[i + 1];

		// 芯の太さ（0.005f）を維持しつつ、左右にクネクネ曲げる
		VertexData lb = { {xB - 0.005f, yBottom, 0.0f, 1.0f}, {0.0f, vBottom}, {0.0f, 0.0f, -1.0f} };
		VertexData lt = { {xT - 0.005f, yTop,    0.0f, 1.0f}, {0.0f, vTop},    {0.0f, 0.0f, -1.0f} };
		VertexData rb = { {xB + 0.005f, yBottom, 0.0f, 1.0f}, {1.0f, vBottom}, {0.0f, 0.0f, -1.0f} };
		VertexData rt = { {xT + 0.005f, yTop,    0.0f, 1.0f}, {1.0f, vTop},    {0.0f, 0.0f, -1.0f} };

		vertexData[vIdx++] = lt; vertexData[vIdx++] = rt; vertexData[vIdx++] = lb;
		vertexData[vIdx++] = rt; vertexData[vIdx++] = rb; vertexData[vIdx++] = lb;
	}

	lightningVertexResource_->Unmap(0, nullptr);
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