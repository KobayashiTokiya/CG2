#pragma once
#include "WinApp.h"
#include "Matrix.h"
#include "Vector.h"
#include "Transform.h"

#include <string>
#include <vector>

#include <d3d12.h>
#include <wrl.h>

#include <fstream>
#include <sstream>
#include <cassert>

class Object3dCommon;

//3Dオブジェクト
class Object3d
{
public://インナークラス
#pragma region モデルデータ用構造体 (ファイル読み込み用)
	//頂点データ
	struct VertexData
	{
		Vector4 position;
		Vector2 texcoord;
		Vector3 normal;
	};
	//マテリアルデータ（ファイルパス）
	struct  MaterialData
	{
		std::string textureFilePath;
		uint32_t textureIndex = 0;
	};
	//モデルデータ
	struct ModelData
	{
		std::vector<VertexData>vertices;
		MaterialData material;
	};
#pragma endregion

#pragma region 定数バッファ用構造体 (GPUへ送る用)
	//マテリアルデータ（GPU用）
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

	//平行光源
	struct DirectionalLight
	{
		Vector4 color;
		Vector3 direction;
		float intensity;
	};
#pragma endregion

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

private:// 非公開メンバ関数
	void CreateVertexData();           //頂点データ作成用の関数
	void CreateMaterialData();         //マテリアルデータ作成用の関数
	void CreateTransformationData();   //座標変換行列データ作成用の関数
	void CreateDirectionalLightData(); //平行光源データ作成用の関数

private://
	Object3dCommon* object3dCommon = nullptr;

	//Objファイルのデータ
	ModelData modelData;

#pragma region バッファリソース関連
	// --- 頂点データ ---
	//バッファリソース
	Microsoft::WRL::ComPtr<ID3D12Resource>vertexResource;
	//バッファリソース内のデータを指すポインタ
	VertexData* vertexData = nullptr;
	//バッファリソースの使い道を補足するバッファビュー
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;

	// --- マテリアル ---
	//バッファリソース（マテリアル用）
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource;
	//バッファリソース内のデータを指すポインタ
	Material* materialData = nullptr;

	// --- 座標変換行列 ---
	//バッファリソース(座標変換行列用)
	Microsoft::WRL::ComPtr<ID3D12Resource>transformationResource;
	//バッファリソース内のデータを指すポインタ
	TransformationMatrix* transformationMatrixData = nullptr;

	// --- 平行光源 ---
	//バッファリソース(平行光源)
	Microsoft::WRL::ComPtr<ID3D12Resource>directionalLightResource;
	//バッファリソース内のデータを指すポインタ
	DirectionalLight* directionalLightData = nullptr;
#pragma endregion

	Transform transform;
	Transform cameraTransform;
};

