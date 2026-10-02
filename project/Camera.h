#pragma once
#include "WinApp.h"
#include "Matrix.h"
#include "Vector.h"
#include "Transform.h"

#include <string>

class Input;

// カメラ
class Camera
{
public://カメラモード
	enum class Mode
	{
		Normal,
		TopDown,
		BottomUp,
		Debug
	};

	void SetMode(Mode mode) { mode_ = mode; }
	Mode GetMode() const { return mode_; }

public://メンバ変数に
	//更新
	void Update();

	// 自由移動用の更新関数
	void DebugUpdate(Input* input);

	//ターゲット(プレイヤー)を追従する更新処理
	void TargetUpdate(const Vector3& targetPosition);

	//上からのカメラ用の更新処理
	void TopDownUpdate(const Vector3& targetPosition);

	//下からのカメラ用
	void BottomUpUpdate(const Vector3& targetPosition);

	//コントラスト
	Camera();
public:
	//setter
	void SetRotate(const Vector3& rotate) { transform.rotate = rotate; }
	void SetTranslate(const Vector3& translate) { transform.translate = translate; }
	void SetFovY(float fovY) { this->fovY = fovY; }
	void SetAspectRatio(float aspectRatio) { this->aspectRatio = aspectRatio; }
	void SetNearClip(float nearClip) { this->nearClip = nearClip; }
	void SetFarClip(float farClip) { this->farClip = farClip; }

	//getter
	const Matrix4x4& GetWorldMatrix()const { return worldMatrix; }
	const Matrix4x4& GetViewMatrix()const { return viewMatrix; }
	const Matrix4x4& GetProjectionMatrix()const { return projectionMatrix; }
	const Matrix4x4& GetViewProjectionMatrix()const { return viewProjectionMatrix; }
	const Vector3& GetRotate()const { return transform.rotate; }
	const Vector3& GetTranslate() const { return transform.translate; }

	Vector3& GetRotateRef() { return transform.rotate; }
	Vector3& GetTranslateRef() { return transform.translate; }
private:
	Transform transform;
	Matrix4x4 worldMatrix;
	Matrix4x4 viewMatrix;

	Matrix4x4 projectionMatrix;
	float fovY;          //水平方向視野角
	float aspectRatio; //アスペクト比
	float nearClip;     //ニアクリップ距離
	float farClip;      //ファークリップ距離

	Matrix4x4 viewProjectionMatrix;

	//デバック用速度設定
	float moveSpeed = 0.2f;
	float rotateSpeed = 0.02f;

	//三人称用
	Vector3 offset_ = { 0.0f,10.0f,-25.0f };

	Mode mode_ = Mode::Normal;
	float topDownHeight_ = 30.0f;
};

