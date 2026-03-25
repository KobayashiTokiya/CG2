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
	void Initialize(DirectXCommon* dxCommon);

	uint32_t Allocate();

	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(uint32_t index);
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(uint32_t index);

	//SRV生成(テクスチャ用)
	void CreateSRVforTexture2D(uint32_t srvIndex, ID3D12Resource* pResource, DXGI_FORMAT Format, UINT MipLevels);
	//SRV生成(Structured Buffer用)
	void CreateSRVforStructuredBuffer(uint32_t srvIndex, ID3D12Resource* pResource, UINT numElements, UINT structureByteStride);

	void PreDraw();

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

