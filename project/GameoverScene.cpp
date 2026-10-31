#include "GameoverScene.h"
#include "Input.h"
#include "SceneManager.h"
#include "FadeManager.h"
#include "TextureManager.h"
#include "Sprite.h"
#include "SpriteCommon.h"
#include "DirectXCommon.h"
#include "SrvManager.h"
#include "ModelManager.h"
#include "Object3d.h"
#include "Object3dCommon.h"
#include "Camera.h"

#include <cmath>
#include <cstdlib>

void GameoverScene::Initialize()
{
	// 1. テクスチャのロード ＆ インデックス／ハンドルの取得
	TextureManager::GetInstance()->LoadTexture("Resource/white.png");
	TextureManager::GetInstance()->LoadTexture("Resource/coin(-500).png");
	TextureManager::GetInstance()->LoadTexture("Resource/rostock_laage_airport_4k.dds");
	TextureManager::GetInstance()->LoadTexture("Resource/floor.png");
	TextureManager::GetInstance()->LoadTexture("Resource/UI/reset.png");

	//スプライトで使用
	backGroundHandle_ = TextureManager::GetInstance()->GetSrvHandleGPU("Resource/white.png");
	
	
	//モデルで使用
	uint32_t whiteTexIndex = TextureManager::GetInstance()->GetSrvIndex("Resource/white.png");
	uint32_t envTexIndex = TextureManager::GetInstance()->GetSrvIndex("Resource/rostock_laage_airport_4k.dds");
	uint32_t floorTexture = TextureManager::GetInstance()->GetSrvIndex("Resource/floor.png");
	uint32_t coinTexture = TextureManager::GetInstance()->GetSrvIndex("Resource/coin(-500).png");

	// ★ 2. カメラの生成と位置・注視点の設定
	camera_ = new Camera();
	camera_->SetMode(Camera::Mode::Normal);
	camera_->SetTranslate({ 0.0f, 4.0f, -15.0f }); // 後ろに引く
	camera_->TargetUpdate({ 0.0f, 3.0f, 0.0f });   // 空中と地面の中間を注視
	Object3dCommon::GetInstance()->SetDefaultCamera(camera_);

	// ★ 3. 3Dモデルのロード・生成・テクスチャ設定
	ModelManager::GetInstance()->LoadModel("gameover.obj");
	ModelManager::GetInstance()->LoadModel("Coin.obj");
	ModelManager::GetInstance()->LoadModel("plane.obj");
	
	groundModel_ = new Object3d();
	groundModel_->Initialize();
	groundModel_->SetModel("plane.obj");
	groundModel_->SetTextureIndex(floorTexture);

	gameoverModel_ = new Object3d();
	gameoverModel_->Initialize();
	gameoverModel_->SetModel("gameover.obj");
	gameoverModel_->SetTextureIndex(whiteTexIndex);       // ★ テクスチャインデックスをセット
	gameoverModel_->SetEnvironmentTexture(envTexIndex);  // ★ 環境マップをセット

	movePos_ = { 0.0f, 18.0f, 0.0f }; // 高い空中に設置
	moveVelY_ = 0.0f;
	isMoveLanded_ = false;

	shakeTimer_ = 0.0f;

	// 座標・回転・スケールの調整（出ない場合はスケールを 5.0f や 10.0f に大きくしてみてください）
	gameoverModel_->SetTranslate(movePos_);
	gameoverModel_->SetRotate({ 0.0f, 0.0f, 0.0f });
	gameoverModel_->SetScale({ 2.0f, 2.0f, 2.0f });

	//落下コインの配置
	coins_.clear();
	const int kCols = 10; // 横に並べる枚数
	const int kRows = 10; // 縦に並べる段数

	for (int r = 0; r < kRows; ++r)
	{
		for (int c = 0; c < kCols; ++c)
		{
			GameoverCoinEffect coinEff;
			coinEff.object = new Object3d();
			coinEff.object->Initialize();
			coinEff.object->SetModel("Coin.obj");
			coinEff.object->SetTextureIndex(coinTexture);
			coinEff.object->SetEnvironmentTexture(envTexIndex);

			// 空中（Y = 7.0 〜 11.0 付近）にグリッド状で浮かべる
			float x = (c - (kCols - 1) * 0.5f) * 2.2f;
			float y = 10.0f + (r * 1.8f);
			float z = (static_cast<float>(rand() % 100) / 100.0f) * 1.0f - 0.5f;

			coinEff.pos = { x, y, z };
			coinEff.velocity = { 0.0f, 0.0f, 0.0f }; // 始めは静止
			coinEff.rotation = { 0.0f, static_cast<float>(rand() % 360), 0.0f };
			coinEff.rotSpeed = { 0.02f, 0.08f, 0.0f }; // 空中でゆらゆら回転
			coinEff.isFalling = false;                 // 着地前は落ちない
			coinEff.timer = 0.0f;
			coinEff.maxLife = 1.5f + (rand() % 100 / 100.0f);

			coinEff.object->SetTranslate(coinEff.pos);
			coinEff.object->SetRotate(coinEff.rotation);
			coinEff.object->SetScale({ 1.0f, 1.0f, 1.0f });
			coinEff.object->Update();

			coins_.push_back(coinEff);
		}
	}


	//背景スプライトの生成
	backGroundSprite_ = new Sprite();
	backGroundSprite_->Initialize(SpriteCommon::GetInstance(), "Resource/white.png");
	backGroundSprite_->SetSize({ 1280.0f, 720.0f });
	backGroundSprite_->SetPosition({ 0.0f, 0.0f });
	backGroundSprite_->SetColor({ 0.0f, 0.0f, 0.0f, 1.0f });

	// フェードイン（明転）開始
	FadeManager::GetInstance()->StartFadeIn(0.5f);
}

void GameoverScene::Update()
{
	FadeManager::GetInstance()->Update();

	if (backGroundSprite_)
	{
		backGroundSprite_->Update();
	}

	// 地面や背景の変換行列を更新して静止画を作る
	Vector3 floorTranslate = { 0.0f, -0.65f, 0.0f };
	Vector3 floorRotate = { 1.57f, 0.0f, 0.0f };
	Vector3 floorScale = { 50.0f, 50.0f, 0.2f };
	groundModel_->SetTranslate(floorTranslate);
	groundModel_->SetRotate(floorRotate);
	groundModel_->SetScale(floorScale);

	groundModel_->Update();

	
	// -------------------------------------------------------------
	// 1. GAME OVER文字の落下＆ドカンと着地
	// -------------------------------------------------------------
	if (!isMoveLanded_ && gameoverModel_)
	{
		const float gravity = -0.04f;
		moveVelY_ += gravity;
		movePos_.y += moveVelY_;

		// 地面（Y = 0.0f）に着地した瞬間
		if (movePos_.y <= 0.0f)
		{
			movePos_.y = 0.0f;
			moveVelY_ = -moveVelY_ * 0.35f; // バウンド

			if (std::abs(moveVelY_) < 0.05f)
			{
				moveVelY_ = 0.0f;
				isMoveLanded_ = true;
			}

			if (shakeTimer_<=0.0f)
			{
				shakeTimer_=0.25f;
			}

			// POW効果：文字が叩きつけられた衝撃で、空中の全コインを一斉に落下開始！
			for (auto& coin : coins_)
			{
				if (!coin.isFalling)
				{
					coin.isFalling = true;
					// バラバラに広がるように横方向と上方向に少しランダムな初速を与える
					float vx = (static_cast<float>(rand() % 200 - 100) / 100.0f) * 0.08f;
					float vy = (static_cast<float>(rand() % 100) / 100.0f) * 0.1f + 0.05f; // 少し浮いてから落っこちる
					float vz = (static_cast<float>(rand() % 200 - 100) / 100.0f) * 0.08f;
					coin.velocity = { vx, vy, vz };
				}
			}
		}
		gameoverModel_->SetTranslate(movePos_);
	}

	if (gameoverModel_)
	{
		gameoverModel_->Update();
	}

	// -------------------------------------------------------------
	// 2. コインの挙動（POW発動後の落下・バウンド・消滅）
	// -------------------------------------------------------------
	for (auto& coin : coins_)
	{
		if (coin.isDead) continue;

		// 空中待機中はゆらゆら回転のみ
		coin.rotation.y += coin.rotSpeed.y;

		// POW発動後の物理計算
		if (coin.isFalling)
		{
			coin.velocity.y -= 0.02f; // 重力で落下
			coin.pos.x += coin.velocity.x;
			coin.pos.y += coin.velocity.y;
			coin.pos.z += coin.velocity.z;

			// 床（Y = 0.0f）でのバウンド
			if (coin.pos.y <= 0.0f)
			{
				coin.pos.y = 0.0f;
				coin.velocity.y = -coin.velocity.y * 0.5f; // 跳ね返り
				coin.velocity.x *= 0.8f;
				coin.velocity.z *= 0.8f;

				coin.rotation.x += 0.3f;
				coin.rotation.z += 0.3f;
			}

			// タイマーを進めて縮小消滅
			coin.timer += 1.0f / 60.0f;
			float progress = coin.timer / coin.maxLife;
			float scale = 1.0f - progress;
			if (scale < 0.0f) scale = 0.0f;

			if (coin.timer >= coin.maxLife)
			{
				coin.isDead = true;
			}

			coin.object->SetScale({ scale, scale, scale });
		}

		coin.object->SetTranslate(coin.pos);
		coin.object->SetRotate(coin.rotation);
		coin.object->Update();
	}

	// メモリ解放
	coins_.erase(
		std::remove_if(coins_.begin(), coins_.end(), [](GameoverCoinEffect& coin)
			{
				if (coin.isDead)
				{
					delete coin.object;
					return true;
				}
				return false;
			}),
		coins_.end()
	);

	// ★ カメラとモデルの更新
	if (camera_)
	{
		Vector3 cameraPos = { 0.0f,0.0f,0.0f };
		Vector3 targetPos = { 0.0f,0.0f,-2.0f };
		if (shakeTimer_>0)
		{
			shakeTimer_ -= 1.0f / 60.0f;
			float shakeX = (static_cast<float>(rand() % 100 - 50) / 100.0f) * 0.4f;
			float shakeY = (static_cast<float>(rand() % 100 - 50) / 100.0f) * 0.4f;
			cameraPos.x += shakeX;
			cameraPos.y += shakeY;
			targetPos.x += shakeX;
			targetPos.y += shakeY;
		}
		camera_->SetTranslate(cameraPos);
		camera_->TargetUpdate(targetPos); // 原点を注視し続ける
	}
	if (gameoverModel_)
	{
		gameoverModel_->Update();
	}

	// 1. ENTERキーで暗転（フェードアウト）開始
	if (Input::GetInstance()->TriggerKey(DIK_RETURN) && !FadeManager::GetInstance()->IsFading())
	{
		FadeManager::GetInstance()->StartFadeOut(0.5f);
	}
	
	if (Input::GetInstance()->TriggerKey(DIK_R))
	{
		GameoverScene::Initialize();
	}

	// 2. 暗転が完了してからタイトルへ遷移
	if (FadeManager::GetInstance()->GetStatus() == FadeManager::Status::kFadeOut &&
		FadeManager::GetInstance()->IsFadeFinished())
	{
		SceneManager::GetInstance()->ChangeScene("TITLE");
		return;
	}
}

void GameoverScene::Draw()
{
	// 描画前処理
	DirectXCommon::GetInstance()->PreDraw();
	SrvManager::GetInstance()->PreDraw();

	ID3D12GraphicsCommandList* commandList = DirectXCommon::GetInstance()->GetCommandList();

	

	// 2. 深度バッファをクリアして3Dモデルを描画可能にする
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = DirectXCommon::GetInstance()->GetDSVCPUDescriptorHandle(0);
	commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	// 3. 3Dモデルを描画
	Object3dCommon::GetInstance()->CommonDrawSettings();
	groundModel_->Draw();

	if (gameoverModel_)
	{
		gameoverModel_->Draw();
	}

	for (auto& coin:coins_)
	{
		if (!coin.isDead && coin.object)
		{
			coin.object->Draw();
		}
	}

	// 1. 背景スプライトを描画
	SpriteCommon::GetInstance()->CommonDrawSettings();
	if (backGroundSprite_)
	{
		//backGroundSprite_->Draw(commandList, backGroundHandle_);
	}


	// 4. 最前面にフェードを描画
	FadeManager::GetInstance()->Draw();

	// 描画後処理
	DirectXCommon::GetInstance()->PostDraw();
}

void GameoverScene::Finalize()
{
	if (backGroundSprite_)
	{
		delete backGroundSprite_;
		backGroundSprite_ = nullptr;
	}
	

	delete groundModel_;
	groundModel_ = nullptr;
	if (gameoverModel_)
	{
		delete gameoverModel_;
		gameoverModel_ = nullptr;
	}
	if (camera_)
	{
		delete camera_;
		camera_ = nullptr;
	}

	for (auto& coin : coins_)
	{
		if (coin.object)
		{
			delete coin.object;
			coin.object = nullptr;
		}
	}
	coins_.clear();
}