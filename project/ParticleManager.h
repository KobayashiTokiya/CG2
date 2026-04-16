#pragma once
#include "DirectXCommon.h"
#include "SrvManager.h"
#include <random>
#include <list>
#include <unordered_map>
#include <string>
#include <wrl.h>
#include <d3d12.h>

struct VertexParticle
{
	Vector4 position;
};

enum class BlendMode
{
	kBlendModeNormal,   // 通常（半透明）
	kBlendModeAdd,      // 加算合成
	kBlendModeSubtract, // 減算合成
	kBlendModeMultiply, // 乗算合成
	kBlendModeScreen,   // スクリーン合成
	kCountOfBlendMode   // 総数
};

class ParticleManager
{
public:
	//インスタンスを取得する関数
	static ParticleManager* GetInstance();

	//初期化
	void Initialize(DirectXCommon* dxCommon, SrvManager* srvManager);

	D3D12_BLEND_DESC GetBlendDesc(BlendMode mode);
private:
	//ルートシグネチャの作成
	void CreateRootSignature();

	//グラフィックスパイプラインの生成
	void CreateGraphicsPipelineState();

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

	Microsoft::WRL::ComPtr<ID3D12RootSignature>rootSignature_;
	Microsoft::WRL::ComPtr<ID3D12PipelineState>graphicsPipelineState_[static_cast<int>(BlendMode::kCountOfBlendMode)];
};

