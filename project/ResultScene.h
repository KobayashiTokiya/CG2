#pragma once
#include <d3d12.h>
#include "BaseScene.h"
#include "Vector.h"

// 前方宣言
class Sprite;

class ResultScene : public BaseScene
{
public:
	ResultScene() = default;
	~ResultScene() override = default;

	void Initialize() override;
	void Finalize() override;
	void Update() override;
	void Draw() override;

private:
	// スプライトのポインタのみ保持
	Sprite* sprite = nullptr;

	// 全画面表示用パラメータ
	Vector2 spritePosition = { 0.0f, 0.0f };
	Vector2 spriteSize = { 1280.0f, 720.0f };
	Vector4 spriteColor = { 1.0f, 1.0f, 1.0f, 1.0f };
	float spriteRotation = 0.0f;

	//スコア描画
	int score_ = 0;
	static const int kMaxScoreDigits = 6;
	Sprite* scoreSprites_[kMaxScoreDigits] = {};
	D3D12_GPU_DESCRIPTOR_HANDLE numberTexHandles_[10] = {};

	//マイナス
	Sprite* minusSprite_ = nullptr;
	D3D12_GPU_DESCRIPTOR_HANDLE minusTexHandle_{};
};