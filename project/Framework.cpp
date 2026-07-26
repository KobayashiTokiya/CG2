#include "Framework.h"

void Framework::Run()
{
	//初期化
	Initialize();

	//ゲームループ
	while (true)
	{
		//毎フレーム更新
		Update();

		//終了リクエストが来たら抜ける
		if (IsEndRequst())
		{
			break;
		}
		//描画
		Draw();
	}
	//終了
	Finalize();
}

void Framework::Initialize()
{
	// WindowsAPIの初期化
	WinApp::GetInstance()->Initialize();

	// DirectXの初期化
	DirectXCommon::GetInstance()->Initialize(WinApp::GetInstance());
	
	// SRVマネージャーの初期化
	SrvManager::GetInstance()->Initialize(DirectXCommon::GetInstance());

	// 入力クラスの初期化
	Input::GetInstance()->Initialize(WinApp::GetInstance());

	// ImGui
#ifdef USE_IMGUI
	ImGuiManager::GetInstance()->Initialize(WinApp::GetInstance(),DirectXCommon::GetInstance(),SrvManager::GetInstance());
#endif
}

void Framework::Update()
{
	// OSメッセージの処理と終了チェック
	if (WinApp::GetInstance()->ProcessMessage())
	{
		endRequst_ = true;
		return;
	}

	// 入力情報の更新
	Input::GetInstance()->Update();
}

void Framework::Finalize()
{
#ifdef USE_IMGUI
	if (ImGuiManager::GetInstance())
	{
		ImGuiManager::GetInstance()->Finalize();
	}
#endif
	delete sceneFactory_;
}