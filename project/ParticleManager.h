#pragma once
#include "DirectXCommon.h"
#include "SrvManager.h"
#include <random>
#include <list>
#include <unordered_map>
#include <string>
#include <wrl.h>
#include <d3d12.h>
#include "Matrix.h"
#include "Transform.h"
#include "numbers"
#include "Collision.h"

class Camera;

struct Particle
{
	Transform transform;
	Vector3 velocity;
	Vector4 color;
	float lifeTime;
	float currentTime;
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

struct ParticleForGPU
{
	Matrix4x4 WVP;
	Matrix4x4 world;
	Vector4 color;
};

struct VertexData
{
	Vector4 position;
	Vector2 texcoord;
	Vector3 normal;
};

struct Emitter
{
	Transform transform;//エミッタのTransform
	uint32_t count;     //発生数
	float frequency;    //発生頻度
	float frequencyTime;//頻度用時刻
};

struct AccelerationField
{
	Vector3 acceleration; //加速度
	AABB area;            //範囲
};

class ParticleManager
{
public:
	//インスタンスを取得する関数
	static ParticleManager* GetInstance();

	//初期化
	void Initialize(DirectXCommon* dxCommon, SrvManager* srvManager);
	//更新
	void Update(Camera* camera);
	//描画
	void Draw(D3D12_GPU_DESCRIPTOR_HANDLE srvHandleGPU);
	//終了
	void Finalize();

	void DrawImGui();

	D3D12_BLEND_DESC GetBlendDesc(BlendMode mode);
private:
	//ルートシグネチャの作成
	void CreateRootSignature();

	//グラフィックスパイプラインの生成
	void CreateGraphicsPipelineState();

	//頂点バッファを作るよう関数
	void CreateVertexBuffer();

	Particle MakeNewParticle(std::mt19937& randomEngine, const Vector3& translate);

	std::list<Particle> Emit(const Emitter& emitter, std::mt19937& randomEngine);
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
	std::random_device seedGenerator;
	std::mt19937 randomEngine_;

	Microsoft::WRL::ComPtr<ID3D12RootSignature>rootSignature_;
	Microsoft::WRL::ComPtr<ID3D12PipelineState>graphicsPipelineState_[static_cast<int>(BlendMode::kCountOfBlendMode)];

	const uint32_t kNumMaxInstance = 100;
	uint32_t numInstance = 0;
	//メンバ変数として頂点リストを持つようにする
	std::vector<VertexData> vertices_;
	
	//Particle particles_[kNumMaxInstance];
	std::list<Particle> particles;
	
	//ImGui用のパーティクル全体を動かす基準点
	Particle base;

	Microsoft::WRL::ComPtr<ID3D12Resource>instancingResource;
	ParticleForGPU* instancingData = nullptr; //書き込み用のポイント

	//板ポリ用頂点バッファ―とビュー
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};

	//エミッタ
	Emitter emitter{};

	//Field
	AccelerationField accelerationField;
	bool useAccelerationField = false;
};

