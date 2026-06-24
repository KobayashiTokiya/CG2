#include "RenderTexture.h"
#include "DirectXCommon.h"
#include "SrvManager.h"
#include <cassert>

void RenderTexture::Create(DirectXCommon* dxCommon, SrvManager* srvManager, uint32_t width, uint32_t height, DXGI_FORMAT format, const Vector4& clearColor)
{
	assert(dxCommon != nullptr);
	assert(srvManager != nullptr);

	ID3D12Device* device = dxCommon->GetDevice();

	// 1. ヒーププロパティの設定
	D3D12_HEAP_PROPERTIES heapProps{};
	heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;

	// 2. リソース設定（描画先として使えるテクスチャにする）
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resourceDesc.Width = width;
	resourceDesc.Height = height;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.Format = format;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.SampleDesc.Quality = 0;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET; // 👈 描画先として許可

	// 3. クリア値の設定（テクスチャの最適化のために必須）
	D3D12_CLEAR_VALUE clearValue{};
	clearValue.Format = format;
	clearValue.Color[0] = clearColor.x;
	clearValue.Color[1] = clearColor.y;
	clearValue.Color[2] = clearColor.z;
	clearValue.Color[3] = clearColor.w;

	currentState = D3D12_RESOURCE_STATE_RENDER_TARGET;

	HRESULT hr = device->CreateCommittedResource(
		&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		currentState,
		&clearValue,
		IID_PPV_ARGS(&resource)
	);
	assert(SUCCEEDED(hr));

	// 4. RTV (RenderTargetView) の作成
	uint32_t rtvIndex = 2;
	rtvHandle = dxCommon->GetRTVCPUDescriptorHandle(rtvIndex);

	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
	rtvDesc.Format = format;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

	device->CreateRenderTargetView(resource.Get(), &rtvDesc, rtvHandle);

	// 5. SRV (ShaderResourceView) の作成
	srvIndex = srvManager->Allocate();
	srvHandleCPU = srvManager->GetCPUDescriptorHandle(srvIndex);
	srvHandleGPU = srvManager->GetGPUDescriptorHandle(srvIndex);

	srvManager->CreateSRVforTexture2D(srvIndex, resource.Get(), format, 1);
}

void RenderTexture::ChangeState(ID3D12GraphicsCommandList* commandList, D3D12_RESOURCE_STATES nextState)
{
	if (currentState == nextState)
	{
		return;
	}

	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = resource.Get();
	barrier.Transition.StateBefore = currentState; // 現在の状態
	barrier.Transition.StateAfter = nextState;     // 遷移後の状態
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	commandList->ResourceBarrier(1, &barrier);

	// 内部の状態を更新
	currentState = nextState;
}