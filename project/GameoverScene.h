#pragma once
#include <d3d12.h>
#include <vector>
#include "BaseScene.h"
#include "Vector.h"

// 前方宣言
class Sprite;
class Object3d;
class Camera;

struct GameoverCoinEffect
{
	Object3d* object = nullptr;
	Vector3 pos{};
	Vector3 velocity{};
	Vector3 rotation{};
	Vector3 rotSpeed{};
	float timer = 0.0f;
	float maxLife = 3.0f;
	bool isFalling = false;
	bool isDead = false;
};

class GameoverScene : public BaseScene
{
public:
	GameoverScene() = default;
	~GameoverScene() override = default;

	void Initialize() override;
	void Finalize() override;
	void Update() override;
	void Draw() override;

private:
	Sprite* backGroundSprite_ = nullptr;
	D3D12_GPU_DESCRIPTOR_HANDLE backGroundHandle_{};

	Object3d* gameoverModel_ = nullptr;
	Camera* camera_ = nullptr;

	Object3d* groundModel_ = nullptr;

	//文字の落下パラメータ
	Vector3 movePos_ = { 0.0f,15.0f,0.0f };
	float moveVelY_ = 0.0f;
	bool isMoveLanded_ = false;

	//落ちるコイン演出
	std::vector<GameoverCoinEffect> coins_;

	//カメラの演出(シェイク)
	float shakeTimer_ = 0.0f;

	//リセットUI
	Sprite* resetSprite_ = nullptr;
	D3D12_GPU_DESCRIPTOR_HANDLE resetHandle_{};

};