#include "SrvManager.h"
#include "DirectXCommon.h"

const uint32_t SrvManager::kMaxSRVCount = 512;

void SrvManager::Initialize()
{
	//デスクリプタヒープの生成
	descriptorHeap = directXCommon->CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, kMaxSRVCount, true);
	//デスクリプタ1個分のサイズを取得して記録
	descriptorSize = directXCommon->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

uint32_t SrvManager::Allocate()
{
	//上限に達していないチェックしてassert
	assert(useIndex < kMaxSRVCount);

	//returnする番号を一旦記録しておく
	int index = useIndex;
	//次回のために番号を１進める
	useIndex++;
	//上で記録した番号をreturn
	return index;
}