#pragma once
#include <d3d12.h>
#include<wrl.h>

#include "DirectXCommon.h"

class Camera;

class SkyboxCommon
{
public:
	//インスタンスを取得する関数
	static SkyboxCommon* GetInstance();

	SkyboxCommon(const SkyboxCommon&) = delete;
	SkyboxCommon& operator=(const SkyboxCommon&) = delete;
	
	//メンバ関数
	//初期化
	void Initialize(DirectXCommon* dxCommon);
	//共通描画設定
	void CommonDrawSettings(ID3D12GraphicsCommandList* commandList);

	//getterとsetter
	DirectXCommon* GetDxCommon()const { return dxCommon_; }

	//カメラ
	void SetDefaultCamera(Camera* camera) { this->defaultCamera = camera; }
	Camera* GetDefaultCamera()const { return defaultCamera; }

private:
	//ルートシグネチャの作成
	void CreateRootSignature();

	//グラフィックスパイプラインの生成
	void CreateGraphicsPipelineState();

private:
	DirectXCommon* dxCommon_ = nullptr;

	Microsoft::WRL::ComPtr<ID3D12RootSignature>rootSignature;
	Microsoft::WRL::ComPtr<ID3D12PipelineState>graphicsPipelineState;

	Camera* defaultCamera = nullptr;

	SkyboxCommon() = default;
	~SkyboxCommon() = default;
};

