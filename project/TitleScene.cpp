#include "TitleScene.h"
#include "DirectXCommon.h"
#include "SrvManager.h"
#include "TextureManager.h"
#include "ModelManager.h"
#include "ParticleManager.h"
#include "SpriteCommon.h"
#include "Object3dCommon.h"
#include "SkyboxCommon.h"
#include "Input.h"
#include "Camera.h"
#include "Object3d.h"
#include "Skybox.h"
#include "Sprite.h"
#include "RenderTexture.h"
#include "PostProcess.h"
#ifdef USE_IMGUI
#include "ImGuiManager.h"
#endif
#include "SceneManager.h"

void TitleScene::Initialize()
{
	// オフスクリーンレンダリング
	renderTexture = new RenderTexture();
	renderTexture->Create(DirectXCommon::GetInstance(), SrvManager::GetInstance(), 1280, 720, DXGI_FORMAT_R8G8B8A8_UNORM, rtClearColor);

	postProcess = new PostProcess();
	postProcess->Initialize(DirectXCommon::GetInstance());

	// カメラ
	camera = new Camera();
	camera->SetRotate({ 0.0f, 0.0f, 0.0f });
	camera->SetTranslate({ 0.0f, 0.0f, 0.0f });

	// 3Dオブジェクト共通部＆生成
	Object3dCommon::GetInstance();
	Object3dCommon::GetInstance()->SetDefaultCamera(camera);

	// アセットロード
	TextureManager::GetInstance()->LoadTexture("Resource/UI/title.png");
	TextureManager::GetInstance()->LoadTexture("Resource/circle.png");
	TextureManager::GetInstance()->LoadTexture("Resource/gradationLine.png");
	TextureManager::GetInstance()->LoadTexture("Resource/white.png");

	// パーティクル用 GPUハンドルの取得
	ringTexHandle = TextureManager::GetInstance()->GetSrvHandleGPU("Resource/circle.png");
	cylinderTexHandle = TextureManager::GetInstance()->GetSrvHandleGPU("Resource/gradationLine.png");
	sphereTexHandle = TextureManager::GetInstance()->GetSrvHandleGPU("Resource/white.png");
	lightningTexHandle = TextureManager::GetInstance()->GetSrvHandleGPU("Resource/white.png");

	ModelManager::GetInstance()->LoadModel("axis.obj");
	ModelManager::GetInstance()->LoadModel("plane.obj");
	ModelManager::GetInstance()->LoadModel("sphere.obj");

	// スカイボックス
	SkyboxCommon::GetInstance()->SetDefaultCamera(camera);

	std::string skyboxDDSPath = "Resource/rostock_laage_airport_4k.dds";
	TextureManager::GetInstance()->LoadTexture(skyboxDDSPath);
	D3D12_GPU_DESCRIPTOR_HANDLE skyboxSRVHandleGPU = TextureManager::GetInstance()->GetSrvHandleGPU(skyboxDDSPath);

	skybox = new Skybox();
	skybox->Initialize(skyboxSRVHandleGPU);

	// スプライト
	sprite = new Sprite();
	sprite->Initialize(SpriteCommon::GetInstance(), "Resource/UI/title.png");
}

void TitleScene::Update()
{
	// ImGuiのフレーム開始処理
#ifdef USE_IMGUI
	ImGuiManager::GetInstance()->Begin();
	ImGuiManager::GetInstance()->UpdateUI(spritePosition, spriteRotation, spriteSize, spriteColor, spriteSwitch,
		object3dTranslate, object3dRotate, object3dScale,
		cameraTranslate, cameraRotate,
		skydomeSwitch,
		postProcessEnable, effectMode, colorScale,
		score
	);
#endif

	// パラメータの反映
	ParticleManager::GetInstance()->DrawImGui();

	sprite->SetPosition(spritePosition);
	sprite->SetRotation(spriteRotation);
	sprite->SetSize(spriteSize);
	sprite->SetColor(spriteColor);

	camera->DebugUpdate(Input::GetInstance());

	// 各種更新（行列計算など）
	skybox->Update(camera);
	sprite->Update();
	ParticleManager::GetInstance()->Update(camera);

	// ENTERキーを押したら
	if (Input::GetInstance()->TriggerKey(DIK_RETURN))
	{
		// シーン切り換え依頼
		SceneManager::GetInstance()->ChangeScene("GAMEPLAY");
	}
}

void TitleScene::Draw()
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
		// title.png の GPU ハンドルを取得して描画に渡す
		D3D12_GPU_DESCRIPTOR_HANDLE titleTexHandle = TextureManager::GetInstance()->GetSrvHandleGPU("Resource/UI/title.png");
		sprite->Draw(DirectXCommon::GetInstance()->GetCommandList(), titleTexHandle);
	}

	// 5. バックバッファへ描画＆ポストプロセス
	renderTexture->ChangeState(DirectXCommon::GetInstance()->GetCommandList(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

	DirectXCommon::GetInstance()->PreDraw();

	postProcess->Draw(DirectXCommon::GetInstance()->GetCommandList(), renderTexture, postProcessEnable, effectMode, colorScale);

	// ImGui描画
#ifdef USE_IMGUI
	ImGuiManager::GetInstance()->End();
	ImGuiManager::GetInstance()->Draw();
#endif

	// フリップ
	DirectXCommon::GetInstance()->PostDraw();
}

void TitleScene::Finalize()
{
	delete postProcess;
	postProcess = nullptr;

	delete renderTexture;
	renderTexture = nullptr;

	delete sprite;
	sprite = nullptr;

	delete skybox;
	skybox = nullptr;

	delete camera;
	camera = nullptr;
}