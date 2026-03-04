#pragma once
#include "WinApp.h"
#include <Matrix.h>

#include <d3d12.h>
#include <wrl.h>

class Object3dCommon;

//頂点データ
struct VertexData
{
	Vector4 position;
	Vector2 texcoord;
	Vector3 normal;
};

struct  MaterialData
{
	std::string textureFilePath;
};

struct ModelData
{
	std::vector<VertexData>vertices;
	MaterialData material;
};

//マテリアルデータ
struct Material
{
	Vector4 color;
	int32_t enableLighting;
	float padding[3]; // パディング（サイズ調整）
	Matrix4x4 uvTransform;
};

//座標変換行列データ
struct  TransformationMatrix
{
	Matrix4x4 WVP;
	Matrix4x4 World;
};

//3Dオブジェクト
class Object3d
{
public://メンバ関数
	//初期化
	void Initialize(Object3dCommon* object3dCommon);
	//更新
	void Update();
	//描画
	void Draw();

	//.mtlファイルの読み取り
	static MaterialData LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename);

	//.objファイルの読み取り
	static ModelData LoadObjFile(const std::string& directoryPath, const std::string& filename);

private://
	//頂点データ作成用の関数
	void CreateVertexData();

	//マテリアルデータ作成用の関数
	void CreateMaterialData();

	//座標変換行列作成用の関数
	void CreateTransformationData();

private://
	Object3dCommon* object3dCommon = nullptr;

	//Objファイルのデータ
	ModelData modelData;

#pragma region 頂点データ
	//バッファリソース
	Microsoft::WRL::ComPtr<ID3D12Resource>vertexResource;
	//バッファリソース内のデータを指すポインタ
	VertexData* vertexData = nullptr;
	//バッファリソースの使い道を補足するバッファビュー
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
#pragma endregion

#pragma region マテリアル
	//バッファリソース（マテリアル用）
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource;
	//バッファリソース内のデータを指すポインタ
	Material* materialData = nullptr;
#pragma endregion

#pragma region 座標変換行列
	//バッファリソース(座標変換行列用)
	Microsoft::WRL::ComPtr<ID3D12Resource>transformationResource;
	//バッファリソース内のデータを指すポインタ
	TransformationMatrix* transformationMatrixData = nullptr;
#pragma endregion

};

