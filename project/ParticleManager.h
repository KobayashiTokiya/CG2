#pragma once
#include "DirectXCommon.h"
#include "SrvManager.h"
#include <random>

struct VertexParticle
{
	Vector4 position;
};

class ParticleManager
{
public:
	//インスタンスを取得する関数
	static ParticleManager* GetInstance();

	//初期化
	void Initialize(DirectXCommon* dxCommon, SrvManager* srvManager);

private:
	ParticleManager() = default;
	~ParticleManager()= default;
	ParticleManager(const ParticleManager&) = delete;
	ParticleManager& operator=(const ParticleManager&) = delete;

private:
	//メンバ変数に記録するためのポインタ
	DirectXCommon* dxCommon_ = nullptr;
	SrvManager* srvManager_ = nullptr;

	//ランダムエンジン
	std::mt19937 randomEngine_;
};

