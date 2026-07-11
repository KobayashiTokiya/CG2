#pragma once
#include "WinApp.h"
#include "DirectXCommon.h"

class SrvManager;

class ImGuiManager
{
public:
	void Initialize(WinApp* winApp, DirectXCommon* dxCommon, SrvManager* srvManager);

	void Finalize();

private:
	WinApp* winApp_ = nullptr;
	DirectXCommon* dxCommon_ = nullptr;
};

