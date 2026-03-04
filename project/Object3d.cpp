#include "Object3d.h"
#include "Object3dCommon.h"

void Object3d::Initialize(Object3dCommon* object3dCommon)
{
	//引数で受け取ってメンバ変数に記録する
	this->object3dCommon = object3dCommon;

	//モデル読み込み
	modelData = LoadObjFile("Resource", "plane.obj");

	//頂点データを作成している関数を呼び出す
	CreateVertexData();
}

void Object3d::Update()
{

}

void Object3d::Draw()
{

}

void Object3d::CreateVertexData()
{
	DirectXCommon* dxCommon = object3dCommon->GetDxCommon();

	//VertexResourceを作る
	vertexResource = dxCommon->CreateBufferResource(sizeof(VertexData) * 6);

	//VertexBufferViewを作成する(値を設定するだけ)
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
	vertexBufferView.SizeInBytes = sizeof(VertexData)*6;
	vertexBufferView.StrideInBytes = sizeof(VertexData);

	//VertexResourceにデータを書き込むためのアドレスを取得してvertexDataに割り当てる
	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
}

void Object3d::CreateMaterialData()
{
	DirectXCommon* dxCommon = object3dCommon->GetDxCommon();

	//マテリアルリソースを作る
	materialResource = dxCommon->CreateBufferResource(sizeof(Material));

	//マテリアルリソースにデータを書き込むためのアドレスを取得してmaterialDataに割り当てる
	materialResource->Map(0,nullptr,reinterpret_cast<void**>(&materialData));

	//マテリアルデータの初期値を書き込む
	materialData->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	materialData->enableLighting = false;
	materialData->uvTransform = MatrixMath::MakeIdentity4x4();
}

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