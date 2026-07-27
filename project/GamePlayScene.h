#pragma once

#include <d3d12.h>
#include<string>
#include "DirectXCommon.h"
#include "Vector.h"
#include "BaseScene.h"
#include "LevelLoader.h"

// 前方宣言
class Camera;
class Object3d;
class Skybox;
class Sprite;
class RenderTexture;
class PostProcess;

class GamePlayScene:public BaseScene
{
public:
	enum PostEffectType
	{
		None = 0,         // 0: 通常（ポストプロセスなし）
		Grayscale = 1,    // 1: グレースケール
		Vignette = 2,     // 2: ヴィネット
		BoxFilter = 3,    // 3: ボックスフィルタ（ぼかし）
		GaussianFilter = 4// 4: ガウスフィルタ（ぼかし）
	};

public:
	GamePlayScene() = default;
	~GamePlayScene()override = default;

	void Initialize()override;
	void Finalize()override;
	void Update()override;
	void Draw()override;

private:
	// 描画関連ポインタ
	Sprite* sprite = nullptr;
	Object3d* object3d = nullptr;
	Skybox* skybox = nullptr;
	Camera* camera = nullptr;
	RenderTexture* renderTexture = nullptr;
	PostProcess* postProcess = nullptr;
	std::vector<Object3d*> objects3d_;

	// GPUハンドル
	D3D12_GPU_DESCRIPTOR_HANDLE ringTexHandle{};
	D3D12_GPU_DESCRIPTOR_HANDLE cylinderTexHandle{};
	D3D12_GPU_DESCRIPTOR_HANDLE sphereTexHandle{};
	D3D12_GPU_DESCRIPTOR_HANDLE lightningTexHandle{};

	// パラメータ・設定値
	Vector4 rtClearColor = { 0.1f, 0.2f, 0.5f, 1.0f };
	Vector2 spritePosition = { 0.0f, 0.0f };
	float spriteRotation = 0.0f;
	Vector2 spriteSize = { 640.0f, 360.0f };
	Vector4 spriteColor = { 1.0f, 1.0f, 1.0f, 1.0f };
	bool spriteSwitch = false;

	Vector3 object3dTranslate = { 0.0f, 0.0f, 0.0f };
	Vector3 object3dRotate = { 0.0f, 0.0f, 0.0f };
	Vector3 object3dScale = { 1.0f, 1.0f, 1.0f };

	Vector3 cameraTranslate = { 0.0f, 0.0f, -100.0f };
	Vector3 cameraRotate = { 0.0f, 0.0f, 0.0f };

	bool skydomeSwitch = false;
	
	//ポストエフェクト
	bool postProcessEnable = true;         
	int effectMode = PostEffectType::Grayscale;
	Vector3 colorScale = { 1.0f, 1.0f, 1.0f }; 


};

