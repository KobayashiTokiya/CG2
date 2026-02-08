#include "Sprite.h"
#include "SpriteCommon.h"
#include "DirectXCommon.h"
#include "WinApp.h"
#include <Matrix.h>

void Sprite::Initialize(SpriteCommon* spriteCommon, std::string textureFilePath)
{
	//引数で受け取ってメンバ変数に記録する
	this->spriteCommon = spriteCommon;

	//単位行列を書き込んでおく
	textureIndex = TextureManager::GetInstance()->GetTextureIndexByFilePath(textureFilePath);

	//初期サイズを画像の解像度に合わせる
	AdjustTextureSize();

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
	commandList->SetGraphicsRootDescriptorTable(2, TextureManager::GetInstance()->GetSrvHandleGPU(textureIndex));

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

	float left = 0.0f - anchorPoint.x;
	float right = 1.0f - anchorPoint.x;
	float top = 0.0f - anchorPoint.y;
	float bottom = 1.0f - anchorPoint.y;

	//左右反転
	if (isFlipX_)
	{
		left = -left;
		right = -right;
	}
	//上下反転
	if (isFlipY_)
	{
		top = -top;
		bottom = -bottom;
	}

	//画像の元サイズ（メタデータ）を取得
	const DirectX::TexMetadata& metadata =
	TextureManager::GetInstance()->GetMetaData(textureIndex);

	//左上座標とサイズを、0.0～1.0 の比率（UV座標）に変換
	float tex_left = textureLeftTop.x / metadata.width;
	float tex_right = (textureLeftTop.x + textureSize.x) / metadata.width;
	float tex_top = textureLeftTop.y / metadata.height;
	float tex_bottom = (textureLeftTop.y + textureSize.y) / metadata.height;

	// --- 1枚目の三角形 ---

	// ■1枚目の三角形
	// 左下
	vertexData[0].position = { left, bottom, 0.0f, 1.0f };
	vertexData[0].texcoord = { tex_left, tex_bottom };     
	vertexData[0].normal = { 0.0f, 0.0f, -1.0f };

	// 左上
	vertexData[1].position = { left, top, 0.0f, 1.0f };
	vertexData[1].texcoord = { tex_left, tex_top };        
	vertexData[1].normal = { 0.0f, 0.0f, -1.0f };

	// 右下
	vertexData[2].position = { right, bottom, 0.0f, 1.0f };
	vertexData[2].texcoord = { tex_right, tex_bottom };    
	vertexData[2].normal = { 0.0f, 0.0f, -1.0f };

	// ■2枚目の三角形
	// 左上
	vertexData[3].position = { left, top, 0.0f, 1.0f };
	vertexData[3].texcoord = { tex_left, tex_top };        
	vertexData[3].normal = { 0.0f, 0.0f, -1.0f };

	// 右上
	vertexData[4].position = { right, top, 0.0f, 1.0f };
	vertexData[4].texcoord = { tex_right, tex_top };       
	vertexData[4].normal = { 0.0f, 0.0f, -1.0f };

	// 右下
	vertexData[5].position = { right, bottom, 0.0f, 1.0f };
	vertexData[5].texcoord = { tex_right, tex_bottom };    
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

void Sprite::AdjustTextureSize()
{
	// テクスチャメタデータを取得
	const DirectX::TexMetadata& metadata = TextureManager::GetInstance()->GetMetaData(textureIndex);

	// 切り出しサイズを画像の解像度に合わせる
	textureSize.x = static_cast<float>(metadata.width);
	textureSize.y = static_cast<float>(metadata.height);

	// 画像サイズをテクスチャサイズに合わせる
	size = textureSize;
}