#pragma once
#include <cstdint>
#include <d3d12.h>
#include <wrl.h>
#include "Vector.h"
#include "Matrix.h"

#include "TextureManager.h"

class  SpriteCommon;

//頂点データ
struct VertexData
{
	Vector4 position;
	Vector2 texcoord;
	Vector3 normal;
};

//マテリアル
struct Material
{
	Vector4 color;
	int32_t enableLighting;
	float padding[3];
	Matrix4x4 uvTransform;
};

//座標変換行列データ
struct TransformationMatrix
{
	Matrix4x4 WVP;
	Matrix4x4 World;
};

//スプライト
class Sprite
{
public:// メンバ関数
	//初期化
	void Initialize(SpriteCommon* spriteCommon,std::string textureFilePath);

	//更新処理
	void Update();

	//描画処理
	void Draw(ID3D12GraphicsCommandList* commandList, const D3D12_GPU_DESCRIPTOR_HANDLE& textureSrvHandle);

public://ゲッターとセッター
	
	//座標設定
	void SetPosition(const Vector2& position) { this->position = position; }
	const Vector2& GetPosition() const { return position; }

	//回転設定
	float GetRotation() const { return rotation; }
	void SetRotation(float rotation) {this->rotation = rotation; }

	//サイズ設定
	const Vector2& GetSize() const { return size; }
	void SetSize(const Vector2& size) {this->size = size; }

	//色設定（マテリアルの書き換え）
	// Initialize後に呼ばれることを想定しています
	const Vector4& GetColor()const { return materialData->color; }
	void SetColor(const Vector4& color) { materialData->color = color; }

	//ゲッター (Drawで使うのでpublicにする必要はあまりないですが、あってもOK)
	D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView()const { return vertexBufferView; }

private://メンバ関数(内部用)
	
	//頂点データ作成用の関数
	void CreateVertexData();

	//マテリアルデータ作成用の関数
	void CreateMaterialData();

	//座標変換行列リソースを作る関数
	void CreateTransformationMatrixData();

private:
	SpriteCommon* spriteCommon = nullptr;

	//頂点データ
	//バッファリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource;
	Microsoft::WRL::ComPtr<ID3D12Resource>indexResource;

	//バッファリソース内のデータを指すポインタ
	VertexData* vertexData = nullptr;
	uint32_t* indexData = nullptr;

	//バッファリソースの使い道を補足するバッファビュー
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
	D3D12_INDEX_BUFFER_VIEW indexBufferView;

	//マテリアル
	//バッファリソース
	Microsoft::WRL::ComPtr<ID3D12Resource>materialResource;
	//バッファリソース内のデータを指すポインタ
	Material* materialData = nullptr;

	//座標変換行列
	//バッファリソース
	Microsoft::WRL::ComPtr<ID3D12Resource>transformationMatrixResource;
	//バッファリソース内のデータを指すポインタ
	TransformationMatrix* transformationMatrixData = nullptr;

	//スプライトの変形情報（Updateで使用）
	Vector2 position = { 0.0f, 0.0f };
	float rotation = 0.0f;
	Vector2 size = { 640.0f, 360.0f }; // 仮の初期サイズ

	//テクスチャ番号
	uint32_t textureIndex = 0;
};

