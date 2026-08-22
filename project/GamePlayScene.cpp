#include "GamePlayScene.h"
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
#include "CollisionManager.h"

#ifdef USE_IMGUI
#include "ImGuiManager.h"
#endif

#include "Coins.h"
#include <cmath>
#include <cstdlib>
#include <ctime>

void GamePlayScene::Initialize()
{
	// スプライト共通部の初期化
	SpriteCommon::GetInstance()->Initialize(DirectXCommon::GetInstance());

	// オフスクリーンレンダリング
	renderTexture = new RenderTexture();
	renderTexture->Create(DirectXCommon::GetInstance(), SrvManager::GetInstance(), 1280, 720, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, rtClearColor);

	postProcess = new PostProcess();
	postProcess->Initialize(DirectXCommon::GetInstance());

	// カメラ
	camera = new Camera();
	camera->SetRotate({ 0.0f, 0.0f, 0.0f });
	camera->SetTranslate({ 0.0f, 0.0f, -50.0f });

	// 3Dオブジェクト共通部＆生成
	Object3dCommon::GetInstance();
	Object3dCommon::GetInstance()->Initialize(DirectXCommon::GetInstance());
	Object3dCommon::GetInstance()->SetDefaultCamera(camera);

	object3d = new Object3d();
	object3d->Initialize();

	// アセットロード
	TextureManager::GetInstance()->LoadTexture("Resource/monsterBall.png");
	TextureManager::GetInstance()->LoadTexture("Resource/uvChecker.png");
	TextureManager::GetInstance()->LoadTexture("Resource/circle.png");
	TextureManager::GetInstance()->LoadTexture("Resource/gradationLine.png");
	TextureManager::GetInstance()->LoadTexture("Resource/rostock_laage_airport_4k.dds");
	TextureManager::GetInstance()->LoadTexture("Resource/lightning.png");
	TextureManager::GetInstance()->LoadTexture("Resource/white.png");
	TextureManager::GetInstance()->LoadTexture("Resource/CoinShadow.png");
	TextureManager::GetInstance()->LoadTexture("Resource/floor.png");

	uint32_t uvCheckerTexIndex = TextureManager::GetInstance()->GetSrvIndex("Resource/uvChecker.png");
	uint32_t whiteTexIndex = TextureManager::GetInstance()->GetSrvIndex("Resource/white.png");
	uint32_t floorTexIndex = TextureManager::GetInstance()->GetSrvIndex("Resource/floor.png");
	uint32_t envTexIndex = TextureManager::GetInstance()->GetSrvIndex("Resource/rostock_laage_airport_4k.dds");

	ModelManager::GetInstance()->LoadModel("axis.obj");
	ModelManager::GetInstance()->LoadModel("plane.obj");
	ModelManager::GetInstance()->LoadModel("sphere.obj");
	ModelManager::GetInstance()->LoadModel("Coin.obj");
	ModelManager::GetInstance()->LoadModel("player.obj");

	// プレイヤーの生成と初期化
	player_ = std::make_unique<Player>();
	player_->Initialize("player.obj");

	// コインの生成と初期化
	std::srand(static_cast<unsigned int>(std::time(nullptr)));
	auto coin = std::make_unique<Coin100>();
	coin->Initialize("Coin.obj", { 0.0f, 100.0f, 0.0f },{ 1.0f, 1.0f, 1.0f }, 100);
	coins_.push_back(std::move(coin));

	// オブジェクトにモデルをセットする
	object3d->SetModel("plane.obj");
	object3d->SetTextureIndex(floorTexIndex);
	//object3d->SetEnvironmentTexture(envTexIndex);


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
	skybox->Initialize(skyboxSRVHandleGPU);

	// スプライト
	sprite = new Sprite();
	sprite->Initialize(SpriteCommon::GetInstance(), "Resource/uvChecker.png");

	// 1. JSON のロード
	LevelData* levelData = LevelLoader::LoadLevelFile("scene");

	// 2. MESH タイプのデータから Object3d を生成して配置
	for (const auto& objectData : levelData->objects)
	{
		if (objectData.type != "MESH") continue;

		std::string modelFileName = objectData.fileName + ".obj";
		ModelManager::GetInstance()->LoadModel(modelFileName);

		Object3d* newObj = new Object3d();
		newObj->Initialize();
		newObj->SetModel(modelFileName);
		newObj->SetTextureIndex(whiteTexIndex);
		newObj->SetEnvironmentTexture(envTexIndex);

		//JSONで指定されたトランスフォームをセット
		newObj->SetTranslate(objectData.transform.translation);
		newObj->SetRotate(objectData.transform.rotation);
		newObj->SetScale(objectData.transform.scaling);

		objects3d_.push_back(newObj);
	}

	//使わなくなったlevelData は削除
	delete levelData;

}

void GamePlayScene::Update()
{
	// ImGuiのフレーム開始処理
#ifdef USE_IMGUI
	ImGuiManager::GetInstance()->Begin();
	ImGuiManager::GetInstance()->UpdateUI(spritePosition, spriteRotation, spriteSize, spriteColor, spriteSwitch,
		object3dTranslate, object3dRotate, object3dScale,
		cameraTranslate, cameraRotate,
		skydomeSwitch,
		postProcessEnable, effectMode, colorScale,
		score);
	
	if (postProcessEnable)
	{
		if (Input::GetInstance()->TriggerKey(DIK_1)) { effectMode = 0; }
		if (Input::GetInstance()->TriggerKey(DIK_2)) { effectMode = 1; }
		if (Input::GetInstance()->TriggerKey(DIK_3)) { effectMode = 2; }
		if (Input::GetInstance()->TriggerKey(DIK_4)) { effectMode = 3; }
		if (Input::GetInstance()->TriggerKey(DIK_5)) { effectMode = 4; }
	}
#endif
	// プレイヤーの更新
	if (player_)
	{
		player_->Update();
	}

	if (Input::GetInstance()->TriggerKey(DIK_1)) { camera->SetMode(Camera::Mode::Normal); }
	if (Input::GetInstance()->TriggerKey(DIK_2)) { camera->SetMode(Camera::Mode::TopDown); }
	if (Input::GetInstance()->TriggerKey(DIK_3)) { camera->SetMode(Camera::Mode::BottomUp); }

	if (camera && player_)
	{
		switch (camera->GetMode())
		{
		case Camera::Mode::Normal:
			camera->TargetUpdate(player_->GetPosition());
			break;
		case Camera::Mode::TopDown:
			camera->TopDownUpdate(player_->GetPosition());
			break;
		case Camera::Mode::BottomUp:
			camera->BottomUpUpdate(player_->GetPosition());
			break;
		}
	}

	// パラメータの反映
	ParticleManager::GetInstance()->DrawImGui();

	sprite->SetPosition(spritePosition);
	sprite->SetRotation(spriteRotation);
	sprite->SetSize(spriteSize);
	sprite->SetColor(spriteColor);

	//地面
	Vector3 floorTranslate = { 0.0f, -1.0f, 0.0f };   // プレイヤーの足元（Y = -1.0f や 0.0f など）
	Vector3 floorRotate = { 1.57f, 0.0f, 0.0f };   // 板ポリゴンを横に倒して水平にする（X軸に90度/約1.57ラジアン回転）
	Vector3 floorScale = { 50.0f, 50.0f, 0.2f };// XとZ（またはY）を大きく広げる
	object3d->SetTranslate(floorTranslate);
	object3d->SetRotate(floorRotate);
	object3d->SetScale(floorScale);

	//camera->DebugUpdate(Input::GetInstance());
	//camera->Update();
	
	// 各種更新（行列計算など）
	skybox->Update(camera);
	object3d->Update();
	sprite->Update();
	ParticleManager::GetInstance()->Update(camera);

	//カメラモード切替
	if (Input::GetInstance()->TriggerKey(DIK_TAB))
	{
		isDebugCamera_ = !isDebugCamera_;
	}


	// 1. コインの更新 ＆ 当たり判定
	for (auto& coin : coins_)
	{
		if (coin && !coin->IsDead())
		{
			coin->Update();

			if (player_)
			{
				Vector3 pPos = player_->GetPosition();
				Vector3 cPos = coin->GetPosition();

				Vector3 bounceDir{};
				Vector3 pushedPos{};

				// パラメータ調整
				float cupHeight = 2.0f;      // コップの高さ
				float innerRadius = 1.2f;    // ★内径（これを大きくすると影が重なった時に確実に入る）
				float wallThickness = 0.2f;  // 壁の厚み（薄くして外に弾かれにくくする）

				// ★順番を変更: まず「中に入っているか（影が重なっているか）」を判定！
				if (CollisionManager::CheckCupBottom(pPos, cPos, cupHeight, innerRadius))
				{
					score += coin->GetScore();
					coin->OnCollect(); // スコア獲得！
				}
				// 中に入っていない時だけ、側面（フチ）の弾き判定を行う
				else if (CollisionManager::CheckCupWallAndGetBounce(pPos, cPos, cupHeight, innerRadius, wallThickness, bounceDir, pushedPos))
				{
					coin->SetPosition(pushedPos);
					coin->OnBounce(bounceDir);
				}
			}
		}
	}

	// 2. 画面外（地下 -10.0f）に落ちた、またはプレイヤーに触れて IsDead() が true になったコインを削除
	coins_.erase(
		std::remove_if(
			coins_.begin(),
			coins_.end(),
			[](const std::unique_ptr<Coin>& coin)
			{
				return coin->IsDead();
			}
		),
		coins_.end()
	);

	const size_t kMaxCoins = 5;

	// 3. コインが消えたら（＝リストが空になったら）同じ場所で再生成
	while (coins_.size() < kMaxCoins)
	{
		std::unique_ptr<Coin> newCoin=nullptr;
		int randomVal = rand() % 100;

		if (randomVal<15)
		{
			newCoin = std::make_unique<Coin500>();
		}
		else if (randomVal < 45)
		{
			newCoin = std::make_unique<Coin100>();
		}
		else if (randomVal < 80)
		{
			newCoin = std::make_unique<Coin10>();
		}
		else
		{
			newCoin = std::make_unique<CoinMinus500>();
		}
		
		float spawnX = static_cast<float>(rand() % 21 - 10);
		float spawnZ = static_cast<float>(rand() % 21 - 10);
		Vector3 spawnPos = { spawnX ,100.0f,spawnZ };

		// 生成・初期化
		newCoin->Initialize("Coin.obj", spawnPos,{ 1.0f, 1.0f, 1.0f }, 100);
		newCoin->Update();
		coins_.push_back(std::move(newCoin));
	}




	for (auto* obj : objects3d_) { obj->Update(); }
}

void GamePlayScene::Draw()
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

	Object3dCommon::GetInstance()->CommonDrawSettings();
	//for (auto* obj : objects3d_) { obj->Draw(); }

	// プレイヤーの描画
	if (player_)
	{
		player_->Draw();
	}

	//Coin
	for (auto& coin : coins_)
	{
		coin->Draw();
	}

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

	postProcess->Draw(DirectXCommon::GetInstance()->GetCommandList(), renderTexture, postProcessEnable, effectMode, colorScale);

	// ImGui描画
#ifdef USE_IMGUI
	ImGuiManager::GetInstance()->End();
	ImGuiManager::GetInstance()->Draw();
#endif

	// フリップ
	DirectXCommon::GetInstance()->PostDraw();
}

void GamePlayScene::Finalize()
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

	for (auto* obj : objects3d_) { delete obj; }
	objects3d_.clear();
}