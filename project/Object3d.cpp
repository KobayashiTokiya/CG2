#include "Object3d.h"
#include "Object3dCommon.h"
#include "Model.h"
#include "ModelManager.h"

void Object3d::Initialize(Object3dCommon* object3dCommon)
{
	//引数で受け取ってメンバ変数に記録する
	this->object3dCommon = object3dCommon;

	//初期化の呼び出し
	CreateTransformationData();   //座標変換行列データ
	CreateDirectionalLightData(); //平行光源データ

	// --- モデルの初期位置・大きさ ---
	transform.scale = { 1.0f, 1.0f, 1.0f };      // 大きさを1倍（等倍）にする
	transform.rotate = { 0.0f, 0.0f, 0.0f };     // 回転なし
	transform.translate = { 0.0f, 0.0f, 0.0f };  // 原点(0,0,0)に配置

	// --- カメラの初期位置・大きさ ---
	cameraTransform.scale = { 1.0f, 1.0f, 1.0f };
	cameraTransform.rotate = { 0.0f, 0.0f, 0.0f };
	// ★超重要：カメラをZ軸の手前（マイナス方向）に引いて、モデルを映す！
	cameraTransform.translate = { 0.0f, 0.0f, -5.0f };
}

void Object3d::Update()
{
	// TransformからWorldMatrixを作る
	Matrix4x4 worldMatrix = MatrixMath::MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);

	// cameraTransformからcameraMatrixを作る
	Matrix4x4 cameraMatrix = MatrixMath::MakeAffineMatrix(cameraTransform.scale, cameraTransform.rotate, cameraTransform.translate);

	// cameraMatrixからviewMatrixを作る(逆行列にする)
	Matrix4x4 viewMatrix = MatrixMath::Inverse(cameraMatrix);

	// ProjectionMatrixを作って透視投影行列を書き込む
	Matrix4x4 projectionMatrix = MatrixMath::MakePerspectiveFovMatrix(0.45f, 1280.0f / 720.0f, 0.1f, 100.0f);

	// --- 行列の合成と転送 ---

	// transformationMatrixData->WVP = worldMatrix*viewMatrix*projectionMatrix
	Matrix4x4 worldViewProjectionMatrix =MatrixMath::Multiply(worldMatrix, MatrixMath::Multiply(viewMatrix, projectionMatrix));

	// 定数バッファに書き込む
	transformationMatrixData->WVP = worldViewProjectionMatrix;
	// transformationMatrixData->World = worldMatrix
	transformationMatrixData->World = worldMatrix;
}

void Object3d::Draw()
{
	ID3D12GraphicsCommandList* commandList = object3dCommon->GetDxCommon()->GetCommandList();

	// ⭕ Object3dの担当：座標変換行列（位置）と 平行光源（光）のセットだけ！
	// 座標変換行列CBufferの場所を設定 (番号:1)
	commandList->SetGraphicsRootConstantBufferView(1, transformationResource.Get()->GetGPUVirtualAddress());

	// 平行光源CBufferの場所を設定 (番号:3)
	commandList->SetGraphicsRootConstantBufferView(3, directionalLightResource.Get()->GetGPUVirtualAddress());

	//3Dモデルが割り当てられていれば描画する
	if (model)
	{
		model->Draw();
	}
}

#pragma region 初期化用データ作成関数群
void Object3d::CreateTransformationData()
{
	DirectXCommon* dxCommon = object3dCommon->GetDxCommon();

	//座標変換行列リソースを作る
	transformationResource = dxCommon->CreateBufferResource(sizeof(TransformationMatrix));

	//座標変換行列リソースにデータを書き込むためのアドレスを取得してtransformationMatrixDataに割り当てる
	transformationResource->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData));

	//単位行列を書き込んでおく
	transformationMatrixData->WVP = MatrixMath::MakeIdentity4x4();
	transformationMatrixData->World = MatrixMath::MakeIdentity4x4();
}

void Object3d::CreateDirectionalLightData()
{
	DirectXCommon* dxCommon = object3dCommon->GetDxCommon();

	//平行光源リソースを作る
	directionalLightResource = dxCommon->CreateBufferResource(sizeof(DirectionalLight));

	//平行光源リソースにデータを書き込むためのアドレスを取得してdirectionalLightDataに割り当てる
	directionalLightResource->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData));

	//デフォルト値を書き込んでおく
	directionalLightData->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f); // 白色
	directionalLightData->direction = Vector3(0.0f, -1.0f, 0.0f);  // 真下に向いて照らす
	directionalLightData->intensity = 1.0f;                        // 明るさ（1.0が基本）
}
#pragma endregion

void Object3d::SetModel(const std::string& filePath)
{
	//モデルを検索してセットする
	model = ModelManager::GetInstance()->FindModel(filePath);
}