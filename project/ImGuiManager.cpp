#include "ImGuiManager.h"
#ifdef USE_IMGUI

#include "SrvManager.h"
#pragma comment(lib, "imgui.lib")

ImGuiManager* ImGuiManager::GetInstance()
{
	static ImGuiManager instance;
	return &instance;
}

void ImGuiManager::Initialize([[maybe_unused]]WinApp* winApp, [[maybe_unused]]DirectXCommon* dxCommon, [[maybe_unused]]SrvManager* srvManager)
{
	winApp_ = winApp;
	dxCommon_ = dxCommon;
	srvManager_ = srvManager;

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
	//後始末
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void ImGuiManager::Begin()
{

	//ImGuiフレーム開始
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void ImGuiManager::End()
{
	//描画前準備
	ImGui::Render();
}

void ImGuiManager::Draw()
{

	ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommandList();

	if (srvManager_)
	{
		ID3D12DescriptorHeap* ppHeaps[] = { srvManager_->GetDescriptorHeap() };
		commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
	}
	
	//描画コマンドを発行
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList);
}

void ImGuiManager::UpdateUI(
	Vector2& spritePosition, float& spriteRotate, Vector2& spriteSize, Vector4& spriteColor, bool& spriteSwich,
	Vector3& object3dTranslate, Vector3& object3dRotate, Vector3& object3dScale,
	Vector3& cameraTranslate, Vector3& cameraRotate,
	bool& skydomeSwith, bool& postProcessEnable, int& effectMode, Vector3& colorScale
)
{
	// ===============================
	// ImGui
	// UIの構築 (スプライトクラスのセッターを使う)
	// ===============================

		// スプライト用
	ImGui::Begin("Controller"); // ウィンドウのタイトル
	ImGui::Checkbox("SpriteSwitch", &spriteSwich);
	if (spriteSwich)
	{
		ImGui::DragFloat2("Position", &spritePosition.x, 1.0f);	//座標
		ImGui::DragFloat("Rotation", &spriteRotate, 0.01f);	//回転
		ImGui::DragFloat2("Size", &spriteSize.x, 1.0f);		    //サイズ
		ImGui::ColorEdit4("Color", &spriteColor.x);	            //色
	}

	// 3Dモデル用
	ImGui::Text("Object3d");
	ImGui::DragFloat3("Translate", &object3dTranslate.x, 0.01f);
	ImGui::DragFloat3("Rotate", &object3dRotate.x, 0.01f);
	ImGui::DragFloat3("Scale", &object3dScale.x, 0.01f);

	// camera用
	ImGui::Text("Camera");
	ImGui::DragFloat3("Translate", &cameraTranslate.x, 0.01f);
	ImGui::DragFloat3("Rotate", &cameraRotate.x, 0.01f);
	//スカイドーム
	ImGui::Text("Skydome");
	ImGui::Checkbox("Skydome Switch", &skydomeSwith);
	//ポストエフェクト
	ImGui::Text("PostProcess");
	ImGui::Checkbox("PostProcess ON/OFF", &postProcessEnable);

	// ラジオボタンで effectMode の値を 0 〜 4 で切り替えられるように変更
	if (postProcessEnable)
	{
		ImGui::RadioButton("0: Copy (Normal)", &effectMode, 0);
		ImGui::RadioButton("1: Grayscale", &effectMode, 1);
		ImGui::RadioButton("2: Vignette", &effectMode, 2);
		ImGui::RadioButton("3: Box Filter", &effectMode, 3);
		ImGui::RadioButton("4: Gaussian Filter", &effectMode, 4);

		// 必要に応じて調整用パラメータを表示
		ImGui::SliderFloat3("Color Scale (RGB)", &colorScale.x, 0.0f, 1.0f);
	}

	ImGui::End(); // ウィンドウの終わり
}
#endif