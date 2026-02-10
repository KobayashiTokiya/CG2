#pragma once
#include "WinApp.h"
#include <Matrix.h>


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

private://
	Object3dCommon* object3dCommon = nullptr;
};

