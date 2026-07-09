#pragma once
#include "WinApp.h"
#include "DirectXCommon.h"

class SrvManager;

class ImGuiManager
{
public:
	void Initialize(WinApp* winApp, DirectXCommon* dxCommon);

private:
	WinApp* winApp_ = nullptr;
	DirectXCommon* dxCommon_ = nullptr;
};

