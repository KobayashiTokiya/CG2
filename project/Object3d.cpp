#include "Object3d.h"
#include "Object3dCommon.h"
#include "Model.h"
#include "ModelManager.h"
#include "Camera.h"

void Object3d::Initialize()
{
	//初期化の呼び出し
	CreateTransformationData();   //座標変換行列データ
	CreateDirectionalLightData(); //平行光源データ

	// --- モデルの初期位置・大きさ ---
	transform.scale = { 2.0f, 2.0f, 2.0f };      // 大きさを2倍にする
	transform.rotate = { 0.0f, 0.0f, 0.0f };     // 回転なし
	transform.translate = { 0.0f, 0.0f, 0.0f };  // 原点(0,0,0)に配置

	this->camera = Object3dCommon::GetInstance()->GetDefaultCamera();
}

void Object3d::Update()
{
	// TransformからWorldMatrixを作る
	Matrix4x4 worldMatrix = MatrixMath::MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);

	Matrix4x4 worldViewProjectionMatrix;

	if (camera)
	{
		const Matrix4x4& viewProjectionMatrix = camera->GetViewProjectionMatrix();
		worldViewProjectionMatrix = MatrixMath::Multiply(worldMatrix, viewProjectionMatrix);

		// ★ directionalLightData のヌルチェックを追加
		if (directionalLightData)
		{
			Vector3 camPos = camera->GetTranslate();
			directionalLightData->cameraWorldPosition = Vector4(camPos.x, camPos.y, camPos.z, 1.0f);
		}
	}
	else
	{
		worldViewProjectionMatrix = worldMatrix;

		// ★ directionalLightData のヌルチェックを追加
		if (directionalLightData)
		{
			directionalLightData->cameraWorldPosition = Vector4(0.0f, 0.0f, 0.0f, 1.0f);
		}
	}

	// 定数バッファに書き込む
	if (transformationMatrixData)
	{
		transformationMatrixData->WVP = worldViewProjectionMatrix;
		transformationMatrixData->World = worldMatrix;
	}
}

void Object3d::Draw()
{
	// ★ ここを追加（未初期化や生成失敗時は描画をスキップしてクラッシュを防ぐ）
	if (!transformationResource || !directionalLightResource || !model)
	{
		return;
	}

	auto common = Object3dCommon::GetInstance();
	ID3D12GraphicsCommandList* commandList = common->GetDxCommon()->GetCommandList();

	commandList->SetPipelineState(common->GetPipelinestate(blendMode_));

	// 座標変換行列CBufferの場所を設定 (番号:1)
	commandList->SetGraphicsRootConstantBufferView(1, transformationResource->GetGPUVirtualAddress());

	D3D12_GPU_DESCRIPTOR_HANDLE textureGPUHandle = TextureManager::GetInstance()->GetSrvHandleGPU(textureIndex_);
	commandList->SetGraphicsRootDescriptorTable(2, textureGPUHandle);

	// 平行光源CBufferの場所を設定 (番号:3)
	commandList->SetGraphicsRootConstantBufferView(3, directionalLightResource->GetGPUVirtualAddress());

	D3D12_GPU_DESCRIPTOR_HANDLE envTexureGPUHandle = TextureManager::GetInstance()->GetSrvHandleGPU(environmentTextureIndex_);
	commandList->SetGraphicsRootDescriptorTable(4, envTexureGPUHandle);

	// 3Dモデルが割り当てられていれば描画する
	if (model)
	{
		model->Draw();
	}
}

#pragma region 初期化用データ作成関数群
void Object3d::CreateTransformationData()
{
	// ★ sizeof(*transformationMatrixData) で構造体実体のサイズを安全に取得
	size_t sizeInBytes = (sizeof(*transformationMatrixData) + 0xFF) & ~0xFF;
	transformationResource = DirectXCommon::GetInstance()->CreateBufferResource(sizeInBytes);

	if (!transformationResource)
	{
		assert(false && "transformationResource の作成に失敗しました");
		return;
	}

	transformationResource->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData));

	transformationMatrixData->WVP = MatrixMath::MakeIdentity4x4();
	transformationMatrixData->World = MatrixMath::MakeIdentity4x4();
}

void Object3d::CreateDirectionalLightData()
{
	// ★ sizeof(*directionalLightData) とすることで型名違いのコンパイルエラーを完全に防ぎます
	size_t sizeInBytes = (sizeof(*directionalLightData) + 0xFF) & ~0xFF;
	directionalLightResource = DirectXCommon::GetInstance()->CreateBufferResource(sizeInBytes);

	if (!directionalLightResource)
	{
		assert(false && "directionalLightResource の生成に失敗しました");
		return;
	}

	directionalLightResource->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData));

	directionalLightData->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	directionalLightData->direction = { 0.0f, -1.0f, 0.0f };
	directionalLightData->intensity = 1.0f;
}
#pragma endregion

void Object3d::SetModel(const std::string& filePath)
{
	//モデルを検索してセットする
	model = ModelManager::GetInstance()->FindModel(filePath);
}