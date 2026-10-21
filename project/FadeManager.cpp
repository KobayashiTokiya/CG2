#include "FadeManager.h"
#include "Sprite.h"
#include "SpriteCommon.h"
#include "TextureManager.h"
#include "Easing.h"
#include "DirectXCommon.h"

FadeManager* FadeManager::GetInstance()
{
	static FadeManager instance;
	return &instance;
}

void FadeManager::Initialize()
{
	// スプライト未生成時のみ生成
	if (!fadeSprite)
	{
		fadeSprite = new Sprite();
		fadeSprite->Initialize(SpriteCommon::GetInstance(), "Resource/white.png");
		fadeSprite->SetPosition({ 0.0f, 0.0f });
		fadeSprite->SetSize({ 1280.0f, 720.0f });
	}

	fadeSprite->SetColor({ 0.0f, 0.0f, 0.0f, 0.0f });
	status = Status::kNone;
	alpha = 0.0f;
	timer = 0.0f;
	isFinished = false;
}

void FadeManager::StartFadeOut(float setDuration)
{
	// ★ 安全ガード：スプライトが未生成なら初期化を呼ぶ
	if (!fadeSprite)
	{
		Initialize();
	}

	status = Status::kFadeOut;
	duration_ = setDuration;
	timer = 0.0f;
	alpha = 0.0f;
	isFinished = false;

	if (fadeSprite)
	{
		fadeSprite->SetColor({ 0.0f, 0.0f, 0.0f, 0.0f });
	}
}

void FadeManager::StartFadeIn(float setDuration)
{
	// ★ 安全ガード：スプライトが未生成なら初期化を呼ぶ
	if (!fadeSprite)
	{
		Initialize();
	}

	status = Status::kFadeIn;
	duration_ = setDuration;
	timer = 0.0f;
	alpha = 1.0f;
	isFinished = false;

	if (fadeSprite)
	{
		fadeSprite->SetColor({ 0.0f, 0.0f, 0.0f, 1.0f });
	}
}

void FadeManager::Update()
{
	if (status == Status::kNone) return;

	timer += 1.0f / 60.0f;
	float progress = std::clamp(timer / duration_, 0.0f, 1.0f);

	float easedProgress = Easing::EaseInOutQuad(progress);

	if (status == Status::kFadeOut)
	{
		// 0.0f -> 1.0f (透明 -> 真っ黒)
		alpha = Easing::Lerp(0.0f, 1.0f, easedProgress);
	}
	else if (status == Status::kFadeIn)
	{
		// 1.0f -> 0.0f (真っ黒 -> 透明)
		alpha = Easing::Lerp(1.0f, 0.0f, easedProgress);
	}

	if (fadeSprite)
	{
		fadeSprite->SetColor({ 0.0f, 0.0f, 0.0f, alpha });
		fadeSprite->Update();
	}

	if (progress >= 1.0f)
	{
		isFinished = true;
		if (status == Status::kFadeIn)
		{
			status = Status::kNone;
		}
	}
}

void FadeManager::Draw()
{
	if (status == Status::kNone && alpha <= 0.0f) return;

	SpriteCommon::GetInstance()->CommonDrawSettings();
	D3D12_GPU_DESCRIPTOR_HANDLE whiteTexHandle = TextureManager::GetInstance()->GetSrvHandleGPU("Resource/white.png");

	if (fadeSprite)
	{
		fadeSprite->Draw(DirectXCommon::GetInstance()->GetCommandList(), whiteTexHandle);
	}
}

void FadeManager::Finalize()
{
	if (fadeSprite)
	{
		delete fadeSprite;
		fadeSprite = nullptr;
	}
}