#pragma once
#include "DirectXCommon.h"

//3Dモデル共通部
class ModelCommon
{
public://メンバ関数
	//初期化
	void Initialize(DirectXCommon* dxCommon);
	//更新
	void Update();
	//描画
	void Draw();

public://ゲッターとセッター
	DirectXCommon* GetDxCommon()const { return dxCommon_; }

private:
	DirectXCommon* dxCommon_;
};

