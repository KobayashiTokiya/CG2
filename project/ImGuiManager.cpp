#include "ImGuiManager.h"
#include <imgui_impl_win32.h>
#include <imgui_impl_dx12.h>
#include "SrvManager.h"

void ImGuiManager::Initialize(WinApp* winApp, DirectXCommon* dxCommon, SrvManager* srvManager)
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

	uint32_t srvIndex = srvManager->Allocate();

	//DirectX12の初期化
	ImGui_ImplDX12_InitInfo initInfo{};
	initInfo.Device = dxCommon_->GetDevice();
	initInfo.CommandQueue = dxCommon_->GetCommandQueue();
	initInfo.NumFramesInFlight =dxCommon_->GetBackBufferCount();
	initInfo.RTVFormat = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	initInfo.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

	initInfo.UserData = srvManager;
	initInfo.SrvDescriptorHeap = srvManager->GetDescriptorHeap();

	initInfo.SrvDescriptorAllocFn = [](ImGui_ImplDX12_InitInfo* info,
		D3D12_CPU_DESCRIPTOR_HANDLE* out_cpu_handle,
		D3D12_GPU_DESCRIPTOR_HANDLE* out_gpu_handle)
	{
			SrvManager* srvMan = static_cast<SrvManager*>(info->UserData);
			uint32_t index = srvMan->Allocate();
			*out_cpu_handle = srvMan->GetCPUDescriptorHandle(index);
			*out_gpu_handle = srvMan->GetGPUDescriptorHandle(index);
	};
	initInfo.SrvDescriptorFreeFn = [](ImGui_ImplDX12_InitInfo* info, D3D12_CPU_DESCRIPTOR_HANDLE spu_handle, D3D12_GPU_DESCRIPTOR_HANDLE gpu_handle)
	{
	
	};

	ImGui_ImplDX12_Init(&initInfo);
}

void ImGuiManager::Finalize()
{
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}