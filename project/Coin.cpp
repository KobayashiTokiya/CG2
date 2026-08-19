#include "Coin.h"
#include "TextureManager.h"

void Coin::Initialize(const std::string& modelFilePath, const Vector3& position, const Vector3& scale, int score)
{
	position_ = position;
	scale_ = scale;
	score_ = score;
	isDead_ = false;

	// 初期速度（真下にゆっくり落ちる）
	velocity_ = { 0.0f, -0.05f, 0.0f };

	object3d_ = std::make_unique<Object3d>();
	object3d_->Initialize();
	object3d_->SetModel(modelFilePath);

	// デフォルトのテクスチャを割り当て
	uint32_t uvCheckerTexIndex = TextureManager::GetInstance()->GetSrvIndex("Resource/uvChecker.png");
	uint32_t envTexIndex = TextureManager::GetInstance()->GetSrvIndex("Resource/rostock_laage_airport_4k.dds");

	object3d_->SetTextureIndex(uvCheckerTexIndex);
	object3d_->SetEnvironmentTexture(envTexIndex);

	object3d_->SetScale(scale_);

	//影用オブジェクト
	shadowObject3d_ = std::make_unique<Object3d>();
	shadowObject3d_->Initialize();
	shadowObject3d_->SetModel("plane.obj");

	uint32_t shadowTexIndex = TextureManager::GetInstance()->GetSrvIndex("Resource/CoinShadow.png");
	shadowObject3d_->SetTextureIndex(shadowTexIndex);
}

void Coin::Update()
{
	//rotate_.x += rotateSpeed_;
	
	// 1. 速度を位置に反映
	position_.x += velocity_.x;
	position_.y += velocity_.y;
	position_.z += velocity_.z;

	// 2. 慣性抵抗（弾かれた後の横移動を徐々に減速させる）
	velocity_.x *= 0.95f;
	velocity_.z *= 0.95f;

	// 一定の高さまで落ちたら画面外として削除フラグを立てる
	if (position_.y < -10.0f)
	{
		isDead_ = true;
	}

	// Transform の反映
	if (object3d_)
	{
		object3d_->SetTranslate(position_);
		object3d_->SetRotate(rotate_);
		object3d_->SetScale(scale_);
		object3d_->Update();
	}

	// 影の位置を更新（コインの真下の地面に配置）
	if (shadowObject3d_)
	{
		Vector3 shadowPos = { position_.x, -0.98f, position_.z };
		Vector3 shadowRotate = { 1.57f, 0.0f, 0.0f };

		// 高さに応じて影の大きさを変える（高いほど影が小さくなる）
		float heightFactor = (position_.y - (-0.98f)) / 20.0f;
		heightFactor = std::clamp(heightFactor, 0.0f, 1.0f);
		float scaleMultiplier = (1.2f - heightFactor * 0.7f);

		Vector3 shadowScale = { scale_.x * scaleMultiplier, scale_.z * scaleMultiplier, 1.0f };

		shadowObject3d_->SetTranslate(shadowPos);
		shadowObject3d_->SetRotate(shadowRotate);
		shadowObject3d_->SetScale(shadowScale);
		shadowObject3d_->Update();
	}
}

// 壁（フチ）に当たった時に外部（GamePlayScene）から呼ばれる処理
void Coin::OnBounce(const Vector3& bounceDir)
{
	// 弾かれた方向（外向き）に速度を与える
	velocity_.x = bounceDir.x * 0.25f;
	velocity_.z = bounceDir.z * 0.25f;
	velocity_.y = 0.08f; // 少し上にポンッと跳ね上がる
}

void Coin::Draw()
{
	// 影を先に描画
	if (shadowObject3d_ && !isDead_)
	{
		shadowObject3d_->Draw();
	}

	if (object3d_ && !isDead_)
	{
		object3d_->Draw();
	}
}