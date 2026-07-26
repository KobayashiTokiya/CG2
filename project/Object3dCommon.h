#pragma once
#include <d3d12.h>
#include<wrl.h>

#include "DirectXCommon.h"
#include "ParticleManager.h"

class Camera;

//3Dオブジェクト
class Object3dCommon
{
public:
	static Object3dCommon* GetInstance();

	Object3dCommon(const Object3dCommon&) = delete;
	Object3dCommon& operator=(const Object3dCommon&) = delete;

	//メンバ関数
	//初期化
	void Initialize(DirectXCommon* dxCommon);
	//更新
	void Update();
	//描画
	void Draw();

	//共通描画設定
	void CommonDrawSettings();

private:
	//ルートシグネチャの作成
	void CreateRootSignature();

	//グラフィックスパイプラインの生成
	void CreateGraphicsPipelineState();

public://getterとsetter
	DirectXCommon* GetDxCommon()const { return dxCommon_; }

	//カメラ
	void SetDefaultCamera(Camera* camera) { this->defaultCamera = camera; }
	Camera* GetDefaultCamera()const { return defaultCamera; }

	//ブレンドモード
	ID3D12PipelineState* GetPipelinestate(BlendMode mode) { return graphicsPipelineState[static_cast<int>(mode)].Get(); }
	D3D12_BLEND_DESC GetBlendDesc(BlendMode mode);
private:
	Object3dCommon() = default;
	~Object3dCommon() = default;

	DirectXCommon* dxCommon_ = nullptr;

	Microsoft::WRL::ComPtr<ID3D12RootSignature>rootSignature;
	Microsoft::WRL::ComPtr<ID3D12PipelineState>graphicsPipelineState[static_cast<int>(BlendMode::kCountOfBlendMode)];

	Camera* defaultCamera = nullptr;
};

