#include "Sprite.h"
#include "SpriteCommon.h"
#include "DirectXCommon.h"
#include "WinApp.h"
#include <Matrix.h>

void Sprite::Initialize(SpriteCommon* spriteCommon)
{
	//引数で受け取ってメンバ変数に記録する
	this->spriteCommon = spriteCommon;

	// 頂点データ作成関数を呼ぶ
	CreateVertexData();
	
	//マテリアルを作る
	CreateMaterialData();

	//座標変換行列
	CreateTransformationMatrixData(); 
}

void Sprite::Update()
{
	//Transform情報を作る (位置、回転、サイズ)
	Vector3 scale = { size.x, size.y, 1.0f };       // sizeを使用
	Vector3 rotate = { 0.0f, 0.0f, rotation };       // rotationを使用
	Vector3 translate = { position.x, position.y, 0.0f };

	//TransformからWorldMatrixを作る
	// アフィン変換行列の作成 (S * R * T)
	Matrix4x4 worldMatrix = MatrixMath::MakeAffineMatrix(scale, rotate, translate);

	//ViewMatrixを作って単位行列を代入
	// スプライトはカメラの影響を受けない（画面に貼り付く）ので単位行列でOK
	Matrix4x4 viewMatrix = MatrixMath::MakeIdentity4x4();

	//ProjectionMatrixを作って並行投影行列を書き込む
	//画面サイズに合わせて設定します(1280 x 720)
	Matrix4x4 projectionMatrix = MatrixMath::MakeOrthographicMatrix(
		0.0f, 0.0f,
		(float)WinApp::kClientWidth, (float)WinApp::kClientHeight,
		0.0f, 100.0f
	);

	//行列をGPUメモリに書き込む (WVP = World * View * Projection)
	Matrix4x4 worldViewProjectionMatrix = MatrixMath::Multiply(worldMatrix, MatrixMath::Multiply(viewMatrix, projectionMatrix));

	transformationMatrixData->WVP = worldViewProjectionMatrix;
	transformationMatrixData->World = worldMatrix;
}

void Sprite::Draw(ID3D12GraphicsCommandList* commandList, const D3D12_GPU_DESCRIPTOR_HANDLE& textureSrvHandle)
{
	//頂点バッファビューの設定
	commandList->IASetVertexBuffers(0, 1, &vertexBufferView);

	//マテリアルCBufferの場所を設定
	//シェーダーのパラメータ番号0番に設定
	commandList->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());

	//座標変換行列CBufferの場所を設定
	//シェーダーのパラメータ番号1番に設定
	commandList->SetGraphicsRootConstantBufferView(1, transformationMatrixResource->GetGPUVirtualAddress());

	//SRV（テクスチャ）のDescriptorTableを設定
	// シェーダーのパラメータ番号2番に設定
	commandList->SetGraphicsRootDescriptorTable(2, textureSrvHandle);

	//描画！(DrawCall)
	// 6頂点（三角形2枚分）を描画する
	commandList->DrawInstanced(6, 1, 0, 0);
}

void Sprite::CreateVertexData()
{
	DirectXCommon* dxCommon = spriteCommon->GetDxCommon();

	//頂点バッファの生成(6頂点分)

	vertexResource = dxCommon->CreateBufferResource(sizeof(VertexData) * 6);

	//頂点データの書き込み用ポイントを作成
	VertexData* vertexData = nullptr;

	// リソースを書き込める状態にする (Map)
	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));

	// --- 1枚目の三角形 ---
	// 左下 (uv: 0,1) -> 画面では下側 (+0.5f)
	vertexData[0].position = { 0.0f,  1.0f, 0.0f, 1.0f };
	vertexData[0].texcoord = { 0.0f, 1.0f };
	vertexData[0].normal = { 0.0f, 0.0f, -1.0f };
	
	// 左上 (uv: 0,0) -> 画面では上側 (-0.5f)
	vertexData[1].position = { 0.0f,  0.0f, 0.0f, 1.0f };
	vertexData[1].texcoord = { 0.0f, 0.0f };
	vertexData[1].normal = { 0.0f, 0.0f, -1.0f };
	
	// 右下 (uv: 1,1) -> 画面では下側 (+0.5f)
	vertexData[2].position = { 1.0f,  1.0f, 0.0f, 1.0f };
	vertexData[2].texcoord = { 1.0f, 1.0f };
	vertexData[2].normal = { 0.0f, 0.0f, -1.0f };

	// --- 2枚目の三角形 ---
	// 左上 (uv: 0,0) -> 画面では上側 (-0.5f)
	vertexData[3].position = { 0.0f,  0.0f, 0.0f, 1.0f };
	vertexData[3].texcoord = { 0.0f, 0.0f };
	vertexData[3].normal = { 0.0f, 0.0f, -1.0f };
	
	// 右上 (uv: 1,0) -> 画面では上側 (-0.5f)
	vertexData[4].position = { 1.0f,  0.0f, 0.0f, 1.0f };
	vertexData[4].texcoord = { 1.0f, 0.0f };
	vertexData[4].normal = { 0.0f, 0.0f, -1.0f };
	
	// 右下 (uv: 1,1) -> 画面では下側 (+0.5f)
	vertexData[5].position = { 1.0f,  1.0f, 0.0f, 1.0f };
	vertexData[5].texcoord = { 1.0f, 1.0f };
	vertexData[5].normal = { 0.0f, 0.0f, -1.0f };

	// 書き込み終了
	vertexResource->Unmap(0, nullptr);

	// 頂点バッファビューの作成
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
	vertexBufferView.SizeInBytes = sizeof(VertexData) * 6;
	vertexBufferView.StrideInBytes = sizeof(VertexData);
}

void Sprite::CreateMaterialData()
{
	DirectXCommon* dxCommon = spriteCommon->GetDxCommon();

	// マテリアルリソースを作る
	materialResource = dxCommon->CreateBufferResource(sizeof(Material));

	// データを書き込むためのポインタを取得
	materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));

	// マテリアルデータの初期化
	materialData->color = { 1.0f, 1.0f, 1.0f, 1.0f }; // 白
	materialData->enableLighting = false; // ライティング無効
	materialData->uvTransform = MatrixMath::MakeIdentity4x4();
}

void Sprite::CreateTransformationMatrixData()
{
	DirectXCommon* dxCommon = spriteCommon->GetDxCommon();

	// リソースを作る
	transformationMatrixResource = dxCommon->CreateBufferResource(sizeof(TransformationMatrix));

	// データを書き込むためのポインタを取得
	transformationMatrixResource->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData));

	// 初期値を設定
	transformationMatrixData->WVP = MatrixMath::MakeIdentity4x4();
	transformationMatrixData->World = MatrixMath::MakeIdentity4x4();
}