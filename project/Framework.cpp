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
	winApp = new WinApp();
	winApp->Initialize();

	// DirectXの初期化
	dxCommon = new DirectXCommon();
	dxCommon->Initialize(winApp);

	// SRVマネージャーの初期化
	srvManeger = new SrvManager();
	srvManeger->Initialize(dxCommon);

	// 入力クラスの初期化
	input = new Input();
	input->Initialize(winApp);

	// ImGui
#ifdef USE_IMGUI
	imguiManager = new ImGuiManager();
	imguiManager->Initialize(winApp, dxCommon, srvManeger);
#endif
}

void Framework::Update()
{
	// OSメッセージの処理と終了チェック
	if (winApp->ProcessMessage())
	{
		endRequst_ = true;
		return;
	}

	// 入力情報の更新
	input->Update();
}

void Framework::Finalize()
{
#ifdef USE_IMGUI
	if (imguiManager)
	{
		imguiManager->Finalize();
		delete imguiManager;
		imguiManager = nullptr;
	}
#endif

	delete srvManeger;
	srvManeger = nullptr;

	delete input;
	input = nullptr;

	delete dxCommon;
	dxCommon = nullptr;

	delete winApp;
	winApp = nullptr;
}