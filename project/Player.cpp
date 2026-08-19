#include "Player.h"
#include "Input.h"
#include "ModelManager.h"
#include "Model.h"
#include "Object3d.h"

Player::Player()
{
}

Player::~Player()
{
}

void Player::Initialize(const std::string& modelFilePath)
{
	position_ = { 0.0f, 0.0f, 0.0f };
	speed_ = 0.5f;

	// 1. Object3d の生成と初期化
	object3d_ = std::make_unique<Object3d>();
	object3d_->Initialize();

	// 2. モデルとテクスチャのセット
	object3d_->SetModel(modelFilePath);

	uint32_t uvCheckerTexIndex = TextureManager::GetInstance()->GetSrvIndex("Resource/uvChecker.png");
	uint32_t envTexIndex = TextureManager::GetInstance()->GetSrvIndex("Resource/rostock_laage_airport_4k.dds");

	object3d_->SetTextureIndex(uvCheckerTexIndex);
	object3d_->SetEnvironmentTexture(envTexIndex);
}

void Player::Update()
{
	if (Input::GetInstance()->PushKey(DIK_W))
	{
		position_.z += speed_;
	}
	if (Input::GetInstance()->PushKey(DIK_A))
	{
		position_.x -= speed_;
	}
	if (Input::GetInstance()->PushKey(DIK_S))
	{
		position_.z -= speed_;
	}
	if (Input::GetInstance()->PushKey(DIK_D))
	{
		position_.x += speed_;
	}
	if (Input::GetInstance()->PushKey(DIK_SPACE)&&!isJumping_)
	{
		velocityY_ = jumpInitialSpeed_;
		isJumping_ = true;
	}
	if (isJumping_)
	{
		velocityY_ -= gravity_;

		position_.y += velocityY_;
		if (position_.y<=0.0f)
		{
			position_.y = 0.0f;
			velocityY_ = 0.0f;
			isJumping_ = false;
		}
	}
	



	if (object3d_)
	{
		object3d_->SetTranslate(position_);
		object3d_->Update();
	}
}

void Player::Draw()
{
	if (object3d_)
	{
		object3d_->Draw();
	}
}