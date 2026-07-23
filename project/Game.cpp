#include "Game.h"

void Game::Initialize()
{
	Framework::Initialize();

	//ゲームプレイシーンの生成
	scene_ = new GamePlayScene();
	//ゲームプレイシーンの初期化
	scene_->Initialize();
	
	// スプライト共通部の初期化
	SpriteCommon::GetInstance()->Initialize(DirectXCommon::GetInstance());

	// テクスチャマネージャーの初期化
	TextureManager::GetInstance()->Initialize(DirectXCommon::GetInstance(), SrvManager::GetInstance());

	// オフスクリーンレンダリング
	renderTexture = new RenderTexture();
	renderTexture->Create(DirectXCommon::GetInstance(), SrvManager::GetInstance(), 1280, 720, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, rtClearColor);

	postProcess = new PostProcess();
	postProcess->Initialize(DirectXCommon::GetInstance());

	// カメラ
	camera = new Camera();
	camera->SetRotate({ 0.0f, 0.0f, 0.0f });
	camera->SetTranslate({ 0.0f, 0.0f, 0.0f });

	// 3Dオブジェクト共通部＆生成
	Object3dCommon::GetInstance();
	Object3dCommon::GetInstance()->Initialize(DirectXCommon::GetInstance());
	Object3dCommon::GetInstance()->SetDefaultCamera(camera);

	object3d = new Object3d();
	object3d->Initialize();

	// 3Dモデルマネージャーの初期化
	ModelManager::GetInstance()->Initialize(DirectXCommon::GetInstance());

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
	ParticleManager::GetInstance()->Initialize(DirectXCommon::GetInstance(), SrvManager::GetInstance());

	ringTexHandle = TextureManager::GetInstance()->GetSrvHandleGPU("Resource/circle.png");
	cylinderTexHandle = TextureManager::GetInstance()->GetSrvHandleGPU("Resource/gradationLine.png");
	sphereTexHandle = TextureManager::GetInstance()->GetSrvHandleGPU("Resource/white.png");
	lightningTexHandle = TextureManager::GetInstance()->GetSrvHandleGPU("Resource/white.png");

	// スカイボックス
	SkyboxCommon::GetInstance()->Initialize(DirectXCommon::GetInstance());
	SkyboxCommon::GetInstance()->SetDefaultCamera(camera);

	std::string skyboxDDSPath = "Resource/rostock_laage_airport_4k.dds";
	D3D12_GPU_DESCRIPTOR_HANDLE skyboxSRVHandleGPU = TextureManager::GetInstance()->GetSrvHandleGPU(skyboxDDSPath);

	skybox = new Skybox();
	skybox->Initialize( skyboxSRVHandleGPU);

	// スプライト
	sprite = new Sprite();
	sprite->Initialize(spriteCommon, "Resource/uvChecker.png");
}

void Game::Update()
{
	//基底クラスの更新処理
	Framework::Update();
	//シーンの更新処理
	scene_->Update();

	// ImGuiのフレーム開始処理
#ifdef USE_IMGUI
	ImGuiManager::GetInstance()->Begin();
	ImGuiManager::GetInstance()->UpdateUI(spritePosition, spriteRotation, spriteSize, spriteColor, spriteSwitch,
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

	camera->DebugUpdate(Input::GetInstance());

	// 各種更新（行列計算など）
	skybox->Update(camera);
	object3d->Update();
	sprite->Update();
	ParticleManager::GetInstance()->Update(camera);
}

void Game::Draw()
{
	// 描画先の変更・クリア
	renderTexture->ChangeState(DirectXCommon::GetInstance()->GetCommandList(), D3D12_RESOURCE_STATE_RENDER_TARGET);

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = renderTexture->GetRtvHandle();
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = DirectXCommon::GetInstance()->GetDSVCPUDescriptorHandle(0);
	DirectXCommon::GetInstance()->GetCommandList()->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

	float clearColor[4] = { rtClearColor.x, rtClearColor.y, rtClearColor.z, rtClearColor.w };
	DirectXCommon::GetInstance()->GetCommandList()->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
	DirectXCommon::GetInstance()->GetCommandList()->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	D3D12_VIEWPORT viewport{ 0.0f, 0.0f, 1280.0f, 720.0f, 0.0f, 1.0f };
	D3D12_RECT scissor{ 0, 0, 1280, 720 };
	DirectXCommon::GetInstance()->GetCommandList()->RSSetViewports(1, &viewport);
	DirectXCommon::GetInstance()->GetCommandList()->RSSetScissorRects(1, &scissor);

	// SRVヒープの再セット
	SrvManager::GetInstance()->PreDraw();

	// 1. パーティクル描画
	ParticleManager::GetInstance()->Draw(
		camera,
		ringTexHandle,
		cylinderTexHandle,
		sphereTexHandle,
		lightningTexHandle
	);

	// 2. 3Dオブジェクトの描画
	Object3dCommon::GetInstance()->CommonDrawSettings();
	object3d->Draw();

	// 3. スカイボックスの描画
	SkyboxCommon::GetInstance()->CommonDrawSettings(DirectXCommon::GetInstance()->GetCommandList());
	if (skydomeSwitch)
	{
		skybox->Draw();
	}

	// 4. スプライトの描画
	SpriteCommon::GetInstance()->CommonDrawSettings();
	if (spriteSwitch)
	{
		sprite->Draw(DirectXCommon::GetInstance()->GetCommandList(), lightningTexHandle);
	}

	// 5. バックバッファへ描画＆ポストプロセス
	renderTexture->ChangeState(DirectXCommon::GetInstance()->GetCommandList(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

	DirectXCommon::GetInstance()->PreDraw();
	
	//シーン描画
	scene_->Draw();
	
	SrvManager::GetInstance()->PreDraw();

	postProcess->Draw(DirectXCommon::GetInstance()->GetCommandList(), renderTexture, postProcessEnable, effectMode, colorScale);

	// ImGui描画
#ifdef USE_IMGUI
	ImGuiManager::GetInstance()->End();
	ImGuiManager::GetInstance()->Draw();
#endif

	// フリップ
	DirectXCommon::GetInstance()->PostDraw();
}

void Game::Finalize()
{
	delete postProcess;
	postProcess = nullptr;

	delete renderTexture;
	renderTexture = nullptr;

	delete sprite;
	sprite = nullptr;

	delete object3d;
	object3d = nullptr;

	delete skybox;
	skybox = nullptr;

	delete camera;
	camera = nullptr;

	ModelManager::GetInstance()->Finalize();
	TextureManager::GetInstance()->Finalize();

	Framework::Finalize();
	//シーンの終了処理
	scene_->Finalize();
	//シーンの解放
	delete scene_;
}