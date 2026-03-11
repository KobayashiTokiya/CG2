#pragma once
#include "Matrix.h"
#include "Vector.h"

#include <string>
#include <vector>

#include <d3d12.h>
#include <wrl.h>

#include <fstream>
#include <sstream>
#include <cassert>

class ModelCommon;

//3Dモデル
class Model
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
	struct Material
	{
		Vector4 color;
		int32_t enableLighting;
		float padding[3];
		Matrix4x4 uvTransform;
	};
#pragma endregion

public:// メンバ関数
	//初期化
	void Initialize(ModelCommon* modelCommon);
	void Draw();

	//.mtlファイルの読み取り
	static MaterialData LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename);
	//.objファイルの読み取り
	static ModelData LoadObjFile(const std::string& directoryPath, const std::string& filename);

private:// 非公開メンバ関数
	void CreateVertexData();           //頂点データ作成用の関数
	void CreateMaterialData();         //マテリアルデータ作成用の関数

private:
	//ModelCommonのポインタ
	ModelCommon* modelCommon_ = nullptr;
	//Objファイルのデータ
	ModelData modelData_;

#pragma region バッファリソース関連
	// --- 頂点データ ---
// //バッファリソース
	Microsoft::WRL::ComPtr<ID3D12Resource>vertexResource;
	//VertexResourceにデータを書き込むためのポインタ バッファリソース内のデータを指すポインタ
	VertexData* vertexData = nullptr;
	//バッファリソースの使い道を補足するバッファビュー
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;

	// --- マテリアル ---
	//バッファリソース（マテリアル用）
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource;
	//バッファリソース内のデータを指すポインタ
	Material* materialData = nullptr;
#pragma endregion
};

