#pragma once
#include <Windows.h>
#include <cstdint>
#include "externals/imgui/imgui.h"
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM IParam);

class WinApp
{
public:
	//シングルトンインスタントの取得
	static WinApp* GetInstance();

	//コピーコンストラクタ・代入演算子を無効
	WinApp(const WinApp&) = delete;
	WinApp& operator=(const WinApp&) = delete;

	//静的メンバ関数
	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	
	//初期化
	void Initialize();
	//更新
	void Update();
	//終了
	void Finalize();
	//メッセージの処理
	bool ProcessMessage();

public://定数
	//クライアント領域のサイズ
	static const int32_t kClientWidth = 1280;
	static const int32_t kClientHeight = 720;

	//hwndのgetter
	HWND GetHwnd() const { return hwnd; }

	//wcのgetter
	HINSTANCE GetHInstance() const { return wc.hInstance; }

private:
	//コンストラクタとデストラクタをprivateに
	WinApp() = default;
	~WinApp() = default;

	//ウィンドウハンドル
	HWND hwnd = nullptr;

	//ウィンドウクラスの設定
	WNDCLASS wc{};
};	