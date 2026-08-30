#pragma once
#include <string>
#include <memory>
#include "Vector.h"
#include "Object3d.h"

class Coin
{
public:
	Coin() = default;
	virtual ~Coin() = default;

	virtual void Initialize(const std::string& modelFilePath, const Vector3& position, const Vector3& scale, int score=0);
	virtual void Update();
	virtual void Draw();

	// プレイヤーと接触した時の効果（子クラスでオーバーライドする）
	virtual void OnCollect() { isDead_ = true; }

	bool IsDead() const { return isDead_; }
	void SetIsDead(bool isDead) { isDead_ = isDead; }

	// 座標の取得（当たり判定用）
	const Vector3& GetPosition() const { return position_; }

	const Vector3& GetScale() const { return scale_; }

	float GetRadius() const { return scale_.x; }

	int GetScore()const { return score_; }

	void SetTexture(uint32_t texIndex)
	{
		if (object3d_)
		{
			object3d_->SetTextureIndex(texIndex);
		}
	}

	void OnBounce(const Vector3& bounceDir);
	
	void SetPosition(const Vector3& pos) { position_ = pos; }
private:
	Vector3 position_ = { 0.0f, 0.0f, 0.0f };
	Vector3 rotate_ = { 0.0f, 0.0f, 0.0f };
	Vector3 scale_ = { 1.0f, 1.0f, 1.0f };

	// 落下速度
	float fallSpeed_ = 0.05f;  
	
	//回転速度
	Vector3 rotationSpeed_ = { 0.0f, 0.0f, 0.0f };
	
	bool isDead_ = false; 

	int score_ = 0;

	std::unique_ptr<Object3d> object3d_ = nullptr;
	std::unique_ptr<Object3d> shadowObject3d_ = nullptr;

	//はじく用の
	Vector3 velocity_ = { 0.0f, -0.05f, 0.0f };
};

