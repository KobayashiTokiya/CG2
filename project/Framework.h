#pragma once
#include <Windows.h>
#include <string>
#include <d3d12.h>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "Dbghelp.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "dxcompiler.lib")

#include "Vector.h"
#include "Input.h"
#include "WinApp.h"
#include "DirectXCommon.h"
#include "SrvManager.h"
#ifdef USE_IMGUI
#include "ImGuiManager.h"
#endif

class Framework
{
public:
	virtual ~Framework() = default;

	virtual void Initialize();
	virtual void Finalize();
	virtual void Update();
	virtual void Draw()=0;
	
	//実行
	void Run();

	//終了フラグのチェック
	virtual bool IsEndRequst() { return endRequst_; }
protected:
	// エンジン基盤系ポインタ
	WinApp* winApp = nullptr;
	DirectXCommon* dxCommon = nullptr;
	SrvManager* srvManeger = nullptr;
	Input* input = nullptr;

#ifdef USE_IMGUI
	ImGuiManager* imguiManager = nullptr;
#endif
	
	//ゲーム終了フラグ
	bool endRequst_ = false;
};

