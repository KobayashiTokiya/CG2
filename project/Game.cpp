#include "Game.h"

void Game::Initialize()
{
	Framework::Initialize();

	// スプライト共通部の初期化
	spriteCommon = new SpriteCommon();
	spriteCommon->Initialize(dxCommon);

	// テクスチャマネージャーの初期化
	TextureManager::GetInstance()->Initialize(dxCommon, srvManeger);

	// オフスクリーンレンダリング
	renderTexture = new RenderTexture();
	renderTexture->Create(dxCommon, srvManeger, 1280, 720, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, rtClearColor);

	postProcess = new PostProcess();
	postProcess->Initialize(dxCommon);

	// カメラ
	camera = new Camera();
	camera->SetRotate({ 0.0f, 0.0f, 0.0f });
	camera->SetTranslate({ 0.0f, 0.0f, 0.0f });

	// 3Dオブジェクト共通部＆生成
	object3dCommon = new Object3dCommon();
	object3dCommon->Initialize(dxCommon);
	object3dCommon->SetDefaultCamera(camera);

	object3d = new Object3d();
	object3d->Initialize(object3dCommon);

	// 3Dモデルマネージャーの初期化
	ModelManager::GetInstance()->Initialize(dxCommon);

	// アセットロード
	TextureManager::GetInstance()->LoadTexture("Resource/monsterBall.png");
	TextureManager::GetInstance()->LoadTexture("Resource/uvChecker.png");
	TextureManager::GetInstance()->LoadTexture("Resource/circle.png");
	TextureManager::GetInstance()->LoadTexture("Resource/gradationLine.png");
	TextureManager::GetInstance()->LoadTexture("Resource/rostock_laage_airport_4k.dds");
	TextureManager::GetInstance()->LoadTexture("Resource/lightning.png");
	TextureManager::GetInstance()->LoadTexture("Resource/white.png");

	uint32_t uvCheckerTexIndex = TextureManager::GetInstance()->GetSrvIndex("Resource/uvChecker.png");
	uint32_t envTexIndex = TextureManager::GetInstance()->GetSrvIndex("Resource/rostock_laage_airport_4k.dds");

	ModelManager::GetInstance()->LoadModel("axis.obj");
	ModelManager::GetInstance()->LoadModel("plane.obj");
	ModelManager::GetInstance()->LoadModel("sphere.obj");

	// オブジェクトにモデルをセットする
	object3d->SetModel("sphere.obj");
	object3d->SetTextureIndex(uvCheckerTexIndex);
	object3d->SetEnvironmentTexture(envTexIndex);

	// パーティクルマネージャー
	ParticleManager::GetInstance()->Initialize(dxCommon, srvManeger);

	ringTexHandle = TextureManager::GetInstance()->GetSrvHandleGPU("Resource/circle.png");
	cylinderTexHandle = TextureManager::GetInstance()->GetSrvHandleGPU("Resource/gradationLine.png");
	sphereTexHandle = TextureManager::GetInstance()->GetSrvHandleGPU("Resource/white.png");
	lightningTexHandle = TextureManager::GetInstance()->GetSrvHandleGPU("Resource/white.png");

	// スカイボックス
	skyboxCommon = new SkyboxCommon();
	skyboxCommon->Initialize(dxCommon);
	skyboxCommon->SetDefaultCamera(camera);

	std::string skyboxDDSPath = "Resource/rostock_laage_airport_4k.dds";
	D3D12_GPU_DESCRIPTOR_HANDLE skyboxSRVHandleGPU = TextureManager::GetInstance()->GetSrvHandleGPU(skyboxDDSPath);

	skybox = new Skybox();
	skybox->Initialize(skyboxCommon, skyboxSRVHandleGPU);

	// スプライト
	sprite = new Sprite();
	sprite->Initialize(spriteCommon, "Resource/uvChecker.png");
}

void Game::Update()
{
	Framework::Update();

	// ImGuiのフレーム開始処理
#ifdef USE_IMGUI
	imguiManager->Begin();
	imguiManager->UpdateUI(spritePosition, spriteRotation, spriteSize, spriteColor, spriteSwitch,
		object3dTranslate, object3dRotate, object3dScale,
		cameraTranslate, cameraRotate,
		skydomeSwitch,
		postProcessEnable, effectMode, colorScale);
#endif

	// パラメータの反映
	ParticleManager::GetInstance()->DrawImGui();

	sprite->SetPosition(spritePosition);
	sprite->SetRotation(spriteRotation);
	sprite->SetSize(spriteSize);
	sprite->SetColor(spriteColor);

	object3d->SetTranslate(object3dTranslate);
	object3d->SetRotate(object3dRotate);
	object3d->SetScale(object3dScale);

	camera->DebugUpdate(input);

	// 各種更新（行列計算など）
	skybox->Update(camera);
	object3d->Update();
	sprite->Update();
	ParticleManager::GetInstance()->Update(camera);
}

void Game::Draw()
{
	// 描画先の変更・クリア
	renderTexture->ChangeState(dxCommon->GetCommandList(), D3D12_RESOURCE_STATE_RENDER_TARGET);

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = renderTexture->GetRtvHandle();
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = dxCommon->GetDSVCPUDescriptorHandle(0);
	dxCommon->GetCommandList()->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

	float clearColor[4] = { rtClearColor.x, rtClearColor.y, rtClearColor.z, rtClearColor.w };
	dxCommon->GetCommandList()->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
	dxCommon->GetCommandList()->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	D3D12_VIEWPORT viewport{ 0.0f, 0.0f, 1280.0f, 720.0f, 0.0f, 1.0f };
	D3D12_RECT scissor{ 0, 0, 1280, 720 };
	dxCommon->GetCommandList()->RSSetViewports(1, &viewport);
	dxCommon->GetCommandList()->RSSetScissorRects(1, &scissor);

	// SRVヒープの再セット
	srvManeger->PreDraw();

	// 1. パーティクル描画
	ParticleManager::GetInstance()->Draw(
		camera,
		ringTexHandle,
		cylinderTexHandle,
		sphereTexHandle,
		lightningTexHandle
	);

	// 2. 3Dオブジェクトの描画
	object3dCommon->CommonDrawSettings();
	object3d->Draw();

	// 3. スカイボックスの描画
	skyboxCommon->CommonDrawSettings(dxCommon->GetCommandList());
	if (skydomeSwitch)
	{
		skybox->Draw();
	}

	// 4. スプライトの描画
	spriteCommon->CommonDrawSettings();
	if (spriteSwitch)
	{
		sprite->Draw(dxCommon->GetCommandList(), lightningTexHandle);
	}

	// 5. バックバッファへ描画＆ポストプロセス
	renderTexture->ChangeState(dxCommon->GetCommandList(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

	dxCommon->PreDraw();
	srvManeger->PreDraw();

	postProcess->Draw(dxCommon->GetCommandList(), renderTexture, postProcessEnable, effectMode, colorScale);

	// ImGui描画
#ifdef USE_IMGUI
	imguiManager->End();
	imguiManager->Draw();
#endif

	// フリップ
	dxCommon->PostDraw();
}

void Game::Finalize()
{
	delete postProcess;
	postProcess = nullptr;

	delete renderTexture;
	renderTexture = nullptr;

	delete sprite;
	sprite = nullptr;

	delete spriteCommon;
	spriteCommon = nullptr;

	delete object3d;
	object3d = nullptr;

	delete object3dCommon;
	object3dCommon = nullptr;

	delete skybox;
	skybox = nullptr;

	delete skyboxCommon;
	skyboxCommon = nullptr;

	delete camera;
	camera = nullptr;

	ModelManager::GetInstance()->Finalize();
	TextureManager::GetInstance()->Finalize();

	Framework::Finalize();
}