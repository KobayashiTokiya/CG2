#pragma once
#include "WinApp.h"
#include "Matrix.h"
#include "Vector.h"
#include "Transform.h"

#include <string>

#include <d3d12.h>
#include <wrl.h>

class Object3dCommon;
class Model; //Modelクラスを使うための先行宣言
class ModelManager;

//3Dオブジェクト
class Object3d
{
public://インナークラス
#pragma region 定数バッファ用構造体 (GPUへ送る用)
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

	//このオブジェクトにモデルをセットする関数
	void SetModel(Model* model) { this->model = model; }

	void SetModel(const std::string& filePath);
public://セッターとゲッター
	void SetScale(const Vector3& scale) { transform.scale=scale; }
	void SetRotate(const Vector3& rotate) { transform.rotate=rotate; }
	void SetTranslate(const Vector3& translate) { transform.translate=translate; }

	const Vector3& GetScale()const { return transform.scale; }
	const Vector3& GetRotate()const { return transform.rotate; }
	const Vector3& GetTranslate()const { return transform.translate; }


private:// 非公開メンバ関数
	void CreateTransformationData();   //座標変換行列データ作成用の関数
	void CreateDirectionalLightData(); //平行光源データ作成用の関数

private://
	Object3dCommon* object3dCommon = nullptr;

	//このオブジェクトが描画するモデルのポインタ
	Model* model = nullptr;

#pragma region バッファリソース関連
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

