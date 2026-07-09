#include "ImGuiManager.h"
#include <imgui_impl_win32.h>
#include <imgui_impl_dx12.h>
#include "SrvManager.h"

void ImGuiManager::Initialize(WinApp* winApp, DirectXCommon* dxCommon)
{
	winApp_ = winApp;
	dxCommon_ = dxCommon;

	//バージョンチェック
	IMGUI_CHECKVERSION();

	//コンテキストの生成
	ImGui::CreateContext();

	//スタイルの設定
	ImGui::StyleColorsDark();

	//Win32用の初期化
	ImGui_ImplWin32_Init(winApp->GetHwnd());

	//DirectX12の初期化
	ImGui_ImplDX12_InitInfo initInfo{};
	initInfo.Device = dxCommon_->GetDevice();
	initInfo.CommandQueue = dxCommon_->GetCommandQueue();
	initInfo.NumFramesInFlight =dxCommon_->GetBackBufferCount();
	initInfo.RTVFormat = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	initInfo.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

	initInfo.SrvDescriptorAllocFn = [](ImGui_ImplDX12_InitInfo* info, D3D12_CPU_DESCRIPTOR_HANDLE* out_cpu_handle, D3D12_GPU_DESCRIPTOR_HANDLE* out_gpu_hadle)
	{
			SrvManager* srvManager = SrvManager::GetInstance();
			uint32_t index = srvManager->CreateSRVforStructuredBuffer();
			*out_cpu_handle = 
				*out_gpu_handle =
	};

	//フォント用SRV
	initInfo.SrvDescriptorHeap = dxCommon_->GetSRVDescriptorHeap();
	initInfo.LegacySingleSrvCpuDescriptor =dxCommon_->GetSRVCPUDescriptorHandle(0);
	initInfo.LegacySingleSrvGpuDescriptor =dxCommon_->GetSRVGPUDescriptorHandle(0);

	ImGui_ImplDX12_Init(&initInfo);
}