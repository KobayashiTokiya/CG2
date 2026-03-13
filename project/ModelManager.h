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
#include <map>

#include"ModelCommon.h"
class Model;

//テクスチャマネージャー
class ModelManager
{
public:
	//シングルトンインスタンスの取得
	static ModelManager* GetInstance();
	//終了
	void Finalize();
	//初期化
	void Initialize(DirectXCommon* dxCommon);
	// モデルファイルの読み込み
	//<param name="filePath">モデルファイルのパス</param>
	void LoadModel(const std::string& filePath);
	//モデル検索
	Model* FindModel(const std::string& filePath);
private:
	static ModelManager* instance;

	//コンストラクト
	ModelManager() = default;
	//デストラクタ
	~ModelManager() = default;
	//コピーコンストラクタ
	ModelManager(const ModelManager&) = delete;
	//コピー代入演算子
	ModelManager& operator=(const ModelManager) = delete;

	//モデルデータ
	std::map<std::string, std::unique_ptr<Model>>models;

	ModelCommon* modelCommon = nullptr;
};

