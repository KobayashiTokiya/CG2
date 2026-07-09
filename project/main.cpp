#include <Windows.h>
#include <string>
#include <d3d12.h>

#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM IParam);

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"Dbghelp.lib")
#pragma comment(lib,"dxguid.lib")
#pragma comment(lib,"dxcompiler.lib")

#include "Vector.h"
#include "Input.h"
#include "WinApp.h"
#include "DirectXCommon.h"
//スプライト共通部
#include "SpriteCommon.h"
//スプライト
#include "Sprite.h"
#include "TextureManager.h"
//3Dオブジェクト
#include "Object3dCommon.h"
#include "Object3d.h"
//モデル
#include "ModelManager.h"
#include"Model.h"
//カメラ
#include "Camera.h"
//SRVマネージャー
#include "SrvManager.h"
//パーティクル
#include "ParticleManager.h"
//スカイボックス
#include "Skybox.h"
#include "SkyboxCommon.h"
//
#include "RenderTexture.h"
#include "PostProcess.h"
//ImGui
#include "ImGuiManager.h"

// メイン関数
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	//ポインタ
	WinApp* winApp = nullptr;

	//WindowsAPIの初期化
	winApp = new WinApp();
	winApp->Initialize();

	//ポインタ
	DirectXCommon* dxCommon = nullptr;
	//DirectXの初期化
	dxCommon = new DirectXCommon();
	dxCommon->Initialize(winApp);
	
	SrvManager* srvManeger = nullptr;
	//SRVマネージャーの初期化
	srvManeger = new SrvManager();
	srvManeger->Initialize(dxCommon);

	SpriteCommon* spriteCommon = nullptr;
	//スプライト共通部の初期化
	spriteCommon = new SpriteCommon;
	spriteCommon->Initialize(dxCommon);

	//入力クラスの初期化
	Input* input = new Input();
	input->Initialize(winApp);

	//テクスチャマネージャーの初期化
	TextureManager::GetInstance()->Initialize(dxCommon,srvManeger);
	
	// ===============================
	// オフスクリーンレンダリング
	// ===============================
	RenderTexture* renderTexture = new RenderTexture();
	Vector4 rtClearColor = { 0.1f,0.2f,0.5f,1.0f };
	renderTexture->Create(dxCommon, srvManeger, 1280, 720, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, rtClearColor);

	PostProcess* postProcess = new PostProcess();
	postProcess->Initialize(dxCommon);

	// ===============================
	// カメラ
	// ===============================
	Camera* camera = new Camera();
	camera->SetRotate({ 0.0f,0.0f,0.0f });
	camera->SetTranslate({ 0.0f,0.0f,0.0f });

	// ===============================
	// object3d
	// ===============================
	Object3dCommon* object3dCommon = nullptr;
	//3Dオブジェクト共通部の初期化
	object3dCommon = new Object3dCommon;
	object3dCommon->Initialize(dxCommon);
	object3dCommon->SetDefaultCamera(camera);

	Object3d* object3d = new Object3d;
	object3d->Initialize(object3dCommon);

	// ===============================
	// model
	// ===============================
	// 3Dモデルマネージャーの初期化
	ModelManager::GetInstance()->Initialize(dxCommon);

	TextureManager::GetInstance()->LoadTexture("Resource/monsterBall.png");
	TextureManager::GetInstance()->LoadTexture("Resource/uvChecker.png");
	TextureManager::GetInstance()->LoadTexture("Resource/circle.png");
	TextureManager::GetInstance()->LoadTexture("Resource/gradationLine.png");
	TextureManager::GetInstance()->LoadTexture("Resource/rostock_laage_airport_4k.dds"); // スカイボックス兼環境マップ
	TextureManager::GetInstance()->LoadTexture("Resource/lightning.png");
	TextureManager::GetInstance()->LoadTexture("Resource/white.png");

	uint32_t uvCheckerTexIndex = TextureManager::GetInstance()->GetSrvIndex("Resource/uvChecker.png");
	uint32_t monsterBallTexIndex = TextureManager::GetInstance()->GetSrvIndex("Resource/monsterBall.png");
	uint32_t circleTexIndex = TextureManager::GetInstance()->GetSrvIndex("Resource/circle.png");
	uint32_t envTexIndex = TextureManager::GetInstance()->GetSrvIndex("Resource/rostock_laage_airport_4k.dds");

	ModelManager::GetInstance()->LoadModel("axis.obj");
	ModelManager::GetInstance()->LoadModel("plane.obj");
	ModelManager::GetInstance()->LoadModel("sphere.obj");

	//オブジェクトにモデルをセットする
	object3d->SetModel("sphere.obj");
	object3d->SetTextureIndex(uvCheckerTexIndex);
	object3d->SetEnvironmentTexture(envTexIndex);

	// ===============================
	//  particle
	// ===============================
	ParticleManager::GetInstance()->Initialize(dxCommon, srvManeger);

	D3D12_GPU_DESCRIPTOR_HANDLE ringTexHandle = TextureManager::GetInstance()->GetSrvHandleGPU("Resource/circle.png");          // リング用
	D3D12_GPU_DESCRIPTOR_HANDLE cylinderTexHandle = TextureManager::GetInstance()->GetSrvHandleGPU("Resource/gradationLine.png"); // シリンダー用
	D3D12_GPU_DESCRIPTOR_HANDLE sphereTexHandle = TextureManager::GetInstance()->GetSrvHandleGPU("Resource/white.png");     // 球体用
	D3D12_GPU_DESCRIPTOR_HANDLE lightningTexHandle = TextureManager::GetInstance()->GetSrvHandleGPU("Resource/white.png");       // 稲妻用
	// ===============================
	// スカイボックス
	// ===============================
	SkyboxCommon* skyboxCommon = nullptr;
	skyboxCommon = new SkyboxCommon;
	skyboxCommon->Initialize(dxCommon);
	skyboxCommon->SetDefaultCamera(camera);

	std::string skyboxDDSPath = "Resource/rostock_laage_airport_4k.dds";
	D3D12_GPU_DESCRIPTOR_HANDLE skyboxSRVHandleGPU = TextureManager::GetInstance()->GetSrvHandleGPU(skyboxDDSPath);

	Skybox* skybox = new Skybox;
	skybox->Initialize(skyboxCommon, skyboxSRVHandleGPU);

	// ===============================
	// スプライト
	// ===============================
	Sprite* sprite = new Sprite();
	sprite->Initialize(spriteCommon, "Resource/uvChecker.png");

	// ===============================
	// ImGui
	// ===============================
	//ImGuiManager* imguiManager = new ImGuiManager();
	//imguiManager->Initialize();

	Vector2 spritePosition = { 0.0f, 0.0f };
	float spriteRotation = 0.0f;
	Vector2 spriteSize = { 640.0f, 360.0f };
	Vector4 spriteColor = { 1.0f, 1.0f, 1.0f, 1.0f };
	bool spriteSwitch = false;

	// 3Dモデル用のImGui操作変数を準備する
	Vector3 object3dTranslate = { 0.0f, 0.0f, 0.0f };
	Vector3 object3dRotate = { 0.0f, 0.0f, 0.0f };
	Vector3 object3dScale = { 1.0f, 1.0f, 1.0f };

	//カメラ
	Vector3 cameraTranslate = { 0.0f, 0.0f, -10.0f };
	Vector3 cameraRotate = { 0.0f, 0.0f, 0.0f };

	//スカイドーム
	bool skydomeSwitch = false;

	//オフスクリーンレンダリング
	bool postProcessEnable = true;
	int effectMode = 0;
	Vector3 colorScale = {100.0f,0.0f,0.0f};

	// メインループ
	while (winApp->ProcessMessage() == false)
	{
		// 入力情報の更新
		input->Update();


		//ImGuiのフレーム開始処理
		ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		// ===============================
		// ImGui
		// UIの構築 (スプライトクラスのセッターを使う)
		// ===============================
		
		// スプライト用
		ImGui::Begin("Controller"); // ウィンドウのタイトル
		ImGui::Checkbox("SpriteSwitch", &spriteSwitch);
		if (spriteSwitch)
		{
			ImGui::DragFloat2("Position", &spritePosition.x, 1.0f);	//座標
			ImGui::DragFloat("Rotation", &spriteRotation, 0.01f);	//回転
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
		ImGui::DragFloat3("Translate", &cameraTranslate.x,0.01f);
		ImGui::DragFloat3("Rotate", &cameraRotate.x, 0.01f);
		//スカイドーム
		ImGui::Text("Skydome");
		ImGui::Checkbox("Skydome Switch", &skydomeSwitch);
		//ポストエフェクト
		ImGui::Text("PostProcess");
		ImGui::Checkbox("PostProcess ON/OFF", &postProcessEnable);

		// ラジオボタンを2つ並べることで、effectMode の値を 0 と 1 で切り替えられるようにします
		if (postProcessEnable)
		{
			ImGui::RadioButton("Background Color Change", &effectMode, 0);
			ImGui::RadioButton("Grayscale", &effectMode, 1);

			// 現在選択されているモードに応じて、表示するスライダーを完全に切り替える
			if (effectMode == 0)
			{
				ImGui::SliderFloat3("BG Color (RGB)", &colorScale.x, 0.0f, 100.0f);
			}
			else
			{
				ImGui::SliderFloat("Grayscale Strength", &colorScale.x, 0.0f, 100.0f); // X値だけを使う
			}
		}
		
		ImGui::End(); // ウィンドウの終わり

		// パーティクル用
		ParticleManager::GetInstance()->DrawImGui();

		//更新処理
		
		// ===============================
		// ImGuiの値を反映
		// ===============================
		// スプライト
		sprite->SetPosition(spritePosition);
		sprite->SetRotation(spriteRotation);
		sprite->SetSize(spriteSize);
		sprite->SetColor(spriteColor);

		//Object3d
		object3d->SetTranslate(object3dTranslate);
		object3d->SetRotate(object3dRotate);
		object3d->SetScale(object3dScale);

		//カメラ
		camera->DebugUpdate(input);
		cameraTranslate = camera->GetTranslate();
		cameraRotate = camera->GetRotate();

		// ===============================
		// 更新（行列計算など）
		// ===============================
		// スカイボックス
		skybox->Update(camera);
		// object3d
		object3d->Update();
		// スプライト
		sprite->Update();
		//パーティクル
		ParticleManager::GetInstance()->Update(camera);
	
		// =========================================================
		// 描画処理
		// =========================================================
		renderTexture->ChangeState(dxCommon->GetCommandList(), D3D12_RESOURCE_STATE_RENDER_TARGET);

		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = renderTexture->GetRtvHandle();
		D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = dxCommon->GetDSVCPUDescriptorHandle(0);
		dxCommon->GetCommandList()->OMSetRenderTargets(1, &rtvHandle, FALSE,&dsvHandle);

		float clearColor[4] = { rtClearColor.x,rtClearColor.y,rtClearColor.z,rtClearColor.w };
		dxCommon->GetCommandList()->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
		dxCommon->GetCommandList()->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);


		D3D12_VIEWPORT viewport{ 0.0f,0.0f,1280.0f,720.0f,0.0f,1.0f };
		D3D12_RECT scissor{ 0,0,1280,720 };
		dxCommon->GetCommandList()->RSSetViewports(1, &viewport);
		dxCommon->GetCommandList()->RSSetScissorRects(1, &scissor);

		//SrvManagerにSRVヒープをセットしてもらう
		srvManeger->PreDraw();

		// ---------------------------------------------------------
		// 1. パーティクル描画
		// ---------------------------------------------------------
		ParticleManager::GetInstance()->Draw(
			camera,
			ringTexHandle,
			cylinderTexHandle,
			sphereTexHandle,
			lightningTexHandle
		);

		// ---------------------------------------------------------
		// 2. 3Dオブジェクトの描画
		// ---------------------------------------------------------
		//3Dオブジェクトの描画準備。3Dオブジェクトの描画に共通のグラフィックスコマンドを積む
		object3dCommon->CommonDrawSettings();
		object3d->Draw();
		
		// ---------------------------------------------------------
		// 3. スカイボックスの描画
		// ---------------------------------------------------------
		//スカイボックスの描画
		skyboxCommon->CommonDrawSettings(dxCommon->GetCommandList());
		if (skydomeSwitch)
		{
			skybox->Draw();
		}
		
		// ---------------------------------------------------------
		// 4. スプライトの描画 (2Dは3Dより後に描画するのが鉄則です)
		// ---------------------------------------------------------
		// スプライト共通設定（ルートシグネチャ、PSO設定）
		spriteCommon->CommonDrawSettings();
		// スプライト描画
		if (spriteSwitch)
		{
			sprite->Draw(dxCommon->GetCommandList(), lightningTexHandle);
		}
		

		//for (Sprite* pSprite :sprites)
		//{
		//	pSprite->Draw(dxCommon->GetCommandList(), srvHandleGPU);
		//}

		// 1. レンダーテクスチャへの書き込みが終了したので、テクスチャとして読める状態に遷移
		renderTexture->ChangeState(dxCommon->GetCommandList(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

		// 2. 描画先を本来の「画面（バックバッファ）」に切り替えつつ、画面をクリアする
		dxCommon->PreDraw();

		// 3. SRVマネージャーのヒープを再セット
		srvManeger->PreDraw();

		// 4. ポストプロセス用のパイプラインを起動し、レンダーテクスチャの内容を画面に描画
		postProcess->Draw(dxCommon->GetCommandList(), renderTexture,postProcessEnable,effectMode,colorScale);

		// 5. ImGuiの内部コマンド生成と発行
		ImGui::Render();
		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), dxCommon->GetCommandList());

		// 描画後処理（フリップなど）
		dxCommon->PostDraw();
	}
	// ImGuiの終了処理
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
	
	// ===============================
	// 解放処理
	// ===============================
	//オフスクリーンレンダリング
	delete postProcess;
	delete renderTexture;

	// スプライト
	delete sprite;
	delete spriteCommon;

	// object3d
	delete object3dCommon;
	delete object3d;

	// カメラ
	delete camera;

	//パーティクル
	
	//スカイボックス
	delete skybox;
	delete skyboxCommon;

	// model
	// 3Dモデルマネージャーの終了
	ModelManager::GetInstance()->Finalize();
	
	//テクスチャマネージャーの終了
	TextureManager::GetInstance()->Finalize();

	//SRVマネージャー
	delete srvManeger;

	delete dxCommon;
	delete input;
	delete winApp;
	//delete imguiManager;

	return 0;
}