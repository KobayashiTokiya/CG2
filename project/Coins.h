#pragma once
#include "Coin.h"
#include "TextureManager.h"

// 500円コイン
class Coin500 : public Coin
{
public:
	Coin500() = default;
	~Coin500() override = default;

	void Initialize(const std::string& modelFilePath, const Vector3& position, const Vector3& scale, int score) override
	{
		// モデル名, 座標, スケール(2.0f), スコア(500)
		Coin::Initialize(modelFilePath, position, {1.8f, 1.8f, 1.8f}, 500);
		TextureManager::GetInstance()->LoadTexture("Resource/Coin(500).png");
		uint32_t texIndex = TextureManager::GetInstance()->GetSrvIndex("Resource/Coin(500).png");
		SetTexture(texIndex);
	}
};

// 100円コイン
class Coin100 : public Coin
{
public:
	Coin100() = default;
	~Coin100() override = default;

	void Initialize(const std::string& modelFilePath, const Vector3& position, const Vector3& scale, int score) override
	{
		// スケール(1.4f), スコア(100)
		Coin::Initialize(modelFilePath, position, { 1.4f, 1.4f, 1.4f }, 100);
		TextureManager::GetInstance()->LoadTexture("Resource/Coin(100).png");
		uint32_t texIndex = TextureManager::GetInstance()->GetSrvIndex("Resource/Coin(100).png");
		SetTexture(texIndex);

	}
};

// 10円コイン
class Coin10 : public Coin
{
public:
	Coin10() = default;
	~Coin10() override = default;

	void Initialize(const std::string& modelFilePath, const Vector3& position, const Vector3& scale, int score) override
	{
		// スケール(1.0f), スコア(10)
		Coin::Initialize(modelFilePath, position, { 1.0f, 1.0f, 1.0f }, 10);
		TextureManager::GetInstance()->LoadTexture("Resource/Coin(10).png");
		uint32_t texIndex = TextureManager::GetInstance()->GetSrvIndex("Resource/Coin(10).png");
		SetTexture(texIndex);
	}
};

// マイナスコイン
class CoinMinus500: public Coin
{
public:
	CoinMinus500() = default;
	~CoinMinus500() override = default;

	void Initialize(const std::string& modelFilePath, const Vector3& position, const Vector3& scale, int score) override
	{
		// スケール(1.2f), スコア(-500)
		Coin::Initialize(modelFilePath, position, { 1.8f, 1.8f, 1.8f }, -500);
		TextureManager::GetInstance()->LoadTexture("Resource/Coin(-500).png");
		uint32_t texIndex = TextureManager::GetInstance()->GetSrvIndex("Resource/Coin(-500).png");
		SetTexture(texIndex);
	}
};