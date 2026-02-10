#pragma once
#include <d3d12.h>
#include<wrl.h>

#include "DirectXCommon.h"

//3Dオブジェクト
class Object3dCommon
{
public://メンバ関数
	//初期化
	void Initialize(DirectXCommon* dxCommon);
	//更新
	void Update();
	//描画
	void Draw();

	//ルートシグネチャの作成
	void CreateRootSignature();

	//グラフィックスパイプラインの生成
	void CreateGraphicsPipelineState();

	//共通描画設定
	void CommonDrawSettings();

public://getterとsetter
	DirectXCommon* GetDxCommon()const { return dxCommon_; }

private:
	DirectXCommon* dxCommon_ = nullptr;

	Microsoft::WRL::ComPtr<ID3D12RootSignature>rootSignature;
	Microsoft::WRL::ComPtr<ID3D12PipelineState>graphicsPipelineState;
};

