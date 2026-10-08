#pragma once

#include <d3d12.h>
#include<string>
#include "DirectXCommon.h"
#include "Vector.h"
#include "BaseScene.h"

// 前方宣言
class Camera;
class Object3d;
class Skybox;
class Sprite;
class RenderTexture;
class PostProcess;

class TitleScene :public BaseScene
{
public:
	TitleScene() = default;
	~TitleScene()override = default;

	void Initialize()override;
	void Finalize()override;
	void Update()override;
	void Draw()override;
private:
	// 描画関連ポインタ
	Sprite* sprite = nullptr;
	Skybox* skybox = nullptr;
	Camera* camera = nullptr;
	RenderTexture* renderTexture = nullptr;
	PostProcess* postProcess = nullptr;

	// パラメータ・設定値
	Vector4 rtClearColor = { 0.1f, 0.2f, 0.5f, 1.0f };
	Vector2 spritePosition = { 0.0f, 0.0f };
	float spriteRotation = 0.0f;
	Vector2 spriteSize = { 1280.0f, 720.0f };
	Vector4 spriteColor = { 1.0f, 1.0f, 1.0f, 1.0f };
	bool spriteSwitch = false;
	
	Vector3 cameraTranslate = { 0.0f, 0.0f, -10.0f };
	Vector3 cameraRotate = { 0.0f, 0.0f, 0.0f };

	bool skydomeSwitch = true;
	bool postProcessEnable = false;
	int effectMode = 0;
	Vector3 colorScale = { 100.0f, 0.0f, 0.0f };

	//タイトル
	static const int kTitleCount = 3;

	Object3d* titleObjects[kTitleCount] = { nullptr, nullptr, nullptr };

	Vector3 titleTranslates[kTitleCount] = {
		{ -3.0f, 2.0f, 0.0f },
		{ 0.5f, 2.0f, 0.0f },
		{ 3.25f, 2.0f, 0.0f }
	};
	Vector3 titleRotates[kTitleCount] = {
		{ 0.0f, 0.0f, 0.0f },
		{ 0.0f, 0.0f, 0.0f },
		{ 0.0f, 0.0f, 0.0f }
	};
	Vector3 titleScales[kTitleCount] = {
		{ 1.0f, 1.0f, 1.0f },
		{ 1.0f, 1.0f, 1.0f },
		{ 1.0f, 1.0f, 1.0f }
	};

	// タイトルのアニメーション用
	float animationTimer = 0.0f;
	float startY = 15.0f;

	const Vector3 targetTranslates[kTitleCount] = {
		{ -3.0f, 2.0f, 0.0f },
		{ 0.5f, 2.0f, 0.0f  },
		{ 3.25f, 2.0f, 0.0f }
	}; 


	//スタート
	const Vector3 targetStartPos = { 0.0f, -0.7f, 0.0f };
	float startOffscreenX = -20.0f; // 左画面外

	Object3d* startObj =nullptr;
	Vector3 startTranslate = targetStartPos;
	Vector3 startRotate = { 0.0f, 0.0f, 0.0f };
	Vector3 startScale = { 1.0f, 1.0f, 2.0f };

	//エンド
	const Vector3 targetEndPos = { 0.0f, -2.5f, 0.0f };
	float endOffscreenX = 20.0f; // 右画面外
	Object3d* endObj = nullptr;
	Vector3 endTranslate = targetEndPos;
	Vector3 endRotate = { 0.0f, 0.0f, 0.0f };
	Vector3 endScale = { 1.0f, 1.0f, 2.0f };

	//背景で落ちてくるコイン
	static const int kCoinCount = 10;

	Object3d* coinObjects[kCoinCount] = { nullptr };

	Vector3 coinTranslates[kCoinCount] = {};
	Vector3 coinRotates[kCoinCount] = {};
	Vector3 coinScales[kCoinCount] = {};

	float coinFallSpeeds[kCoinCount] = {};
	Vector3 coinRotSpeeds[kCoinCount] = {};

	// メニュー選択状態の定義
	enum class MenuType
	{
		kStart,
		kEnd,
	};
	MenuType currentMenu = MenuType::kStart; // 初期選択はスタート

	// 選択カーソル用コイン
	Object3d* selectCoinObj = nullptr;
	Vector3 selectCoinTranslate = { 0.0f, 0.0f, 0.0f };
	Vector3 selectCoinRotate = { 0.0f, 0.0f, 0.0f };
	Vector3 selectCoinScale = { 0.8f, 0.8f, 0.8f }; // 文字に合わせたサイズ
};

