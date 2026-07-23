#pragma once

#include <d3d12.h>
#include<string>
#include "DirectXCommon.h"

// 前方宣言
class Camera;
class Object3d;
class Skybox;
class Sprite;
class RenderTexture;
class PostProcess;

class GamePlayScene
{
public:
	GamePlayScene() = default;
	~GamePlayScene() = default;

	void Initialize();
	void Finalize();
	void Update();
	void Draw();

private:
	// 描画関連ポインタ
	Sprite* sprite = nullptr;
	Object3d* object3d = nullptr;
	Skybox* skybox = nullptr;
	Camera* camera = nullptr;
	RenderTexture* renderTexture = nullptr;
	PostProcess* postProcess = nullptr;

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
	bool spriteSwitch = true;

	Vector3 object3dTranslate = { 0.0f, 0.0f, 0.0f };
	Vector3 object3dRotate = { 0.0f, 0.0f, 0.0f };
	Vector3 object3dScale = { 1.0f, 1.0f, 1.0f };

	Vector3 cameraTranslate = { 0.0f, 0.0f, -100.0f };
	Vector3 cameraRotate = { 0.0f, 0.0f, 0.0f };

	bool skydomeSwitch = true;
	bool postProcessEnable = true;
	int effectMode = 0;
	Vector3 colorScale = { 100.0f, 0.0f, 0.0f };
};

