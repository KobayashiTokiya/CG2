#pragma once
#ifdef USE_IMGUI
#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx12.h>


#include "WinApp.h"
#include "DirectXCommon.h"
#include "Vector.h"

class SrvManager;

class ImGuiManager
{
public:
	static ImGuiManager* GetInstance();

	ImGuiManager(const ImGuiManager&) = delete;
	ImGuiManager& operator=(const ImGuiManager&) = delete;

	void Initialize(WinApp* winApp, DirectXCommon* dxCommon, SrvManager* srvManager);

	void Finalize();

	//ImGui受付開始
	void Begin();

	//ImGui受付終了
	void End();

	//画面への描画
	void Draw();

	void UpdateUI(
		Vector2& spritePosition, float& spriteRotate, Vector2& spriteSize, Vector4& spriteColor, bool& spriteSwich,
		Vector3& object3dTranslate, Vector3& object3dRotate, Vector3& object3dScale,
		Vector3& cameraTranslate, Vector3& cameraRotate,
		bool& skydomeSwith, bool& postProcessEnable, int& effectMode, Vector3& colorScale,
		int score
	);

	void UpdateTitleUI(
		Vector2& spritePosition, float& spriteRotate, Vector2& spriteSize, Vector4& spriteColor, bool& spriteSwich,
		Vector3 titleTranslates[3], Vector3 titleRotates[3], Vector3 titleScales[3],
		Vector3& startTranslate, Vector3& startRotate, Vector3& startScale,
		Vector3& endTranslate, Vector3& endRotate, Vector3& endScale,
		Vector3& cameraTranslate, Vector3& cameraRotate, bool& skydomeSwith
	);

private:
	ImGuiManager() = default;
	~ImGuiManager() = default;

	WinApp* winApp_ = nullptr;
	DirectXCommon* dxCommon_ = nullptr;
	SrvManager* srvManager_ = nullptr;
};

#endif