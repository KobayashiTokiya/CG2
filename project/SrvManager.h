#pragma once
#include "WinApp.h"
#include "Matrix.h"
#include "Vector.h"
#include "Transform.h"

#include <string>
#include <d3d12.h>
#include <wrl.h>

class DirectXCommon;

//SRV管理
class SrvManager
{
public:
	//初期化
	void Initialize();

	uint32_t Allocate();
private:
	DirectXCommon* directXCommon = nullptr;

	//最大SRV数(最大テクスチャ枚数)
	static const uint32_t kMaxSRVCount;
	//SRV用のデスクリプタサイズ
	uint32_t descriptorSize;
	//SRV用デスクリプタヒープ
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap;

	//次に使用するSRVインデックス
	uint32_t useIndex = 0;

};

