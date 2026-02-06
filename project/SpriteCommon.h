#pragma once
#include <d3d12.h>
#include<wrl.h>

#include "DirectXCommon.h"

//スプライト共通部
class SpriteCommon
{
public://メンバ関数
	//初期化
	void Initialize(DirectXCommon* dxCommon);

	//ルートシグネチャの作成
	void CreateRootSignature();
	
	//グラフィックスパイプラインの生成
	void CreateGraphicsPipelineState();

	//共通描画設定
	void CommonDrawSettings();

public://ゲッター
	DirectXCommon* GetDxCommon()const { return dxCommon_; }


private:
	DirectXCommon* dxCommon_ = nullptr;

	Microsoft::WRL::ComPtr<ID3D12RootSignature>rootSignature;
	Microsoft::WRL::ComPtr<ID3D12PipelineState>graphicsPipelineState;

	
};

