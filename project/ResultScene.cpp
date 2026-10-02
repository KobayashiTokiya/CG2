#include "ResultScene.h"
#include "DirectXCommon.h"
#include "SrvManager.h"
#include "TextureManager.h"
#include "SpriteCommon.h"
#include "Sprite.h"
#include "Input.h"
#include "SceneManager.h"
#include "ScoreManager.h"

void ResultScene::Initialize()
{
	score_=ScoreManager::GetScore();

	TextureManager::GetInstance()->LoadTexture("Resource/UI/score.png");

	//背景
	sprite = new Sprite();
	sprite->Initialize(SpriteCommon::GetInstance(), "Resource/UI/score.png");
	sprite->SetPosition(spritePosition);
	sprite->SetSize(spriteSize);
	sprite->SetRotation(spriteRotation);
	sprite->SetColor(spriteColor);

	// 3. 数字テクスチャの読み込み & 数字スプライトの生成
	Vector2 basePos = { 700.0f, 300.0f }; 
	float digitWidth = 48.0f;            // 1桁あたりの幅

	for (int i = 0; i < 10; ++i)
	{
		std::string path = "Resource/number/" + std::to_string(i) + ".png";
		TextureManager::GetInstance()->LoadTexture(path);
		numberTexHandles_[i] = TextureManager::GetInstance()->GetSrvHandleGPU(path);
	}

	for (int i = 0; i < kMaxScoreDigits; ++i)
	{
		scoreSprites_[i] = new Sprite();
		scoreSprites_[i]->Initialize(SpriteCommon::GetInstance(), "Resource/number/0.png");
		scoreSprites_[i]->SetSize({ 48.0f, 96.0f });
		scoreSprites_[i]->SetPosition({ basePos.x - (i * digitWidth), basePos.y });
	}

	// マイナスの初期化
	TextureManager::GetInstance()->LoadTexture("Resource/number/minus.png");
	minusTexHandle_ = TextureManager::GetInstance()->GetSrvHandleGPU("Resource/number/minus.png");
	minusSprite_ = new Sprite();
	minusSprite_->Initialize(SpriteCommon::GetInstance(), "Resource/number/minus.png");
	minusSprite_->SetSize({ 48.0f, 96.0f });
}

void ResultScene::Update()
{
	// 背景の更新
	if (sprite) 
	{ 
		sprite->Update();
	};

	// ENTERキーでタイトルへ遷移
	if (Input::GetInstance()->TriggerKey(DIK_RETURN))
	{
		SceneManager::GetInstance()->ChangeScene("TITLE");
	}
}

void ResultScene::Draw()
{
	ID3D12GraphicsCommandList* commandList = DirectXCommon::GetInstance()->GetCommandList();

	// 1. SRVヒープのセット
	SrvManager::GetInstance()->PreDraw();

	// 2. スプライトの描画共通設定
	SpriteCommon::GetInstance()->CommonDrawSettings();
	if (sprite)
	{
		D3D12_GPU_DESCRIPTOR_HANDLE resultTexHandle = TextureManager::GetInstance()->GetSrvHandleGPU("Resource/UI/score.png");
		sprite->Draw(commandList, resultTexHandle);
	}

	// -------------------------------------------------
	// スコアの描画処理
	// -------------------------------------------------
	bool isMinus = (score_ < 0);
	int tempScore = std::abs(score_);
	int activeDigits = 1;
	int checkVal = tempScore;

	Vector2 basePos = { 700.0f, 300.0f };
	float digitWidth = 48.0f;

	while (checkVal >= 10)
	{
		checkVal /= 10;
		activeDigits++;
	}

	// 各桁の数字描画
	for (int i = 0; i < kMaxScoreDigits; ++i)
	{
		int digit = tempScore % 10;
		tempScore /= 10;

		if (i >= activeDigits) { continue; }

		if (scoreSprites_[i])
		{
			if (isMinus) { scoreSprites_[i]->SetColor({ 1.0f, 0.3f, 0.3f, 1.0f }); }
			else { scoreSprites_[i]->SetColor({ 1.0f, 1.0f, 1.0f, 1.0f }); }
			scoreSprites_[i]->Update();
			scoreSprites_[i]->Draw(commandList, numberTexHandles_[digit]);
		}
	}

	// ★ マイナス記号の描画（forループの外で1度だけ実行）
	if (isMinus && minusSprite_)
	{
		minusSprite_->SetPosition({ basePos.x - (activeDigits * digitWidth), basePos.y });
		minusSprite_->SetColor({ 1.0f, 0.3f, 0.3f, 1.0f });
		minusSprite_->Update();
		minusSprite_->Draw(commandList, minusTexHandle_);
	}
}

void ResultScene::Finalize()
{
	// メモリ解放
	if (sprite)
	{
		delete sprite;
		sprite = nullptr;
	}


	for (int i = 0; i < kMaxScoreDigits; ++i)
	{
		if (scoreSprites_[i])
		{
			delete scoreSprites_[i];
			scoreSprites_[i] = nullptr;
		}
	}

	if (minusSprite_)
	{
		delete minusSprite_;
		minusSprite_ = nullptr;
	}
}