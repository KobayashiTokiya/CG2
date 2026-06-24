#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <cstdint>

#include "Engine/math/Vector.h"

// 依存クラスの前方宣言
class DirectXCommon;
class SrvManager;

class RenderTexture
{
public:
	RenderTexture() = default;
	~RenderTexture() = default;

	// レンダーテクスチャの生成
	void Create(DirectXCommon* dxCommon, SrvManager* srvManager, uint32_t width, uint32_t height, DXGI_FORMAT format, const Vector4& clearColor);

	// リソースの状態（State）を遷移させる関数
	void ChangeState(ID3D12GraphicsCommandList* commandList, D3D12_RESOURCE_STATES nextState);

	// ゲッター群
	ID3D12Resource* GetResource() const { return resource.Get(); }
	D3D12_CPU_DESCRIPTOR_HANDLE GetRtvHandle() const { return rtvHandle; }
	D3D12_GPU_DESCRIPTOR_HANDLE GetSrvHandle() const { return srvHandleGPU; }

private:
	Microsoft::WRL::ComPtr<ID3D12Resource> resource;
	D3D12_RESOURCE_STATES currentState = D3D12_RESOURCE_STATE_RENDER_TARGET; // 現在の状態を追跡

	uint32_t srvIndex = 0;
	D3D12_CPU_DESCRIPTOR_HANDLE srvHandleCPU{};
	D3D12_GPU_DESCRIPTOR_HANDLE srvHandleGPU{};

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle{};
};