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
#include "Easing.h"

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
	camera->SetTranslate({ 0.0f, 0.0f, -20.0f });

	// 3Dオブジェクト共通部＆生成
	Object3dCommon::GetInstance();
	Object3dCommon::GetInstance()->SetDefaultCamera(camera);

	// スカイボックス
	SkyboxCommon::GetInstance()->SetDefaultCamera(camera);

	std::string skyboxDDSPath = "Resource/titleBackground.png";
	TextureManager::GetInstance()->LoadTexture(skyboxDDSPath);
	D3D12_GPU_DESCRIPTOR_HANDLE skyboxSRVHandleGPU = TextureManager::GetInstance()->GetSrvHandleGPU(skyboxDDSPath);

	skybox = new Skybox();
	skybox->Initialize(skyboxSRVHandleGPU);

	// スプライト
	sprite = new Sprite();
	sprite->Initialize(SpriteCommon::GetInstance(), "Resource/UI/title.png");

	// タイトル用テクスチャのロード
	TextureManager::GetInstance()->LoadTexture("Resource/white.png");
	TextureManager::GetInstance()->LoadTexture("Resource/titleBackground.png");

	TextureManager::GetInstance()->LoadTexture("Resource/Coin(500).png");
	TextureManager::GetInstance()->LoadTexture("Resource/Coin(100).png");
	TextureManager::GetInstance()->LoadTexture("Resource/Coin(10).png");
	TextureManager::GetInstance()->LoadTexture("Resource/Coin(-500).png");

	//タイトル用モデルのロード
	ModelManager::GetInstance()->LoadModel("title01.obj");
	ModelManager::GetInstance()->LoadModel("title02.obj");
	ModelManager::GetInstance()->LoadModel("title03.obj");
	ModelManager::GetInstance()->LoadModel("start.obj");
	ModelManager::GetInstance()->LoadModel("end.obj");
	ModelManager::GetInstance()->LoadModel("Coin.obj");

	const std::string modelNames[kTitleCount] = { "title01.obj", "title02.obj", "title03.obj" };
	uint32_t titleTexIndex = TextureManager::GetInstance()->GetSrvIndex("Resource/Coin(500).png");

	for (int i = 0; i < kTitleCount; i++)
	{
		titleObjects[i] = new Object3d();
		titleObjects[i]->Initialize();
		titleObjects[i]->SetModel(modelNames[i]);
		titleObjects[i]->SetTextureIndex(titleTexIndex);
	}

	uint32_t whiteTexIndex = TextureManager::GetInstance()->GetSrvIndex("Resource/white.png");
	startObj = new Object3d();
	startObj->Initialize();
	startObj->SetModel("start.obj");
	startObj->SetTextureIndex(whiteTexIndex);
	startTranslate = targetStartPos;
	startTranslate.x = startOffscreenX;

	endObj = new Object3d();
	endObj->Initialize();
	endObj->SetModel("end.obj");
	endObj->SetTextureIndex(whiteTexIndex);
	endTranslate = targetEndPos;
	endTranslate.x = endOffscreenX;

	// カメラ初期位置設定と行列更新
	camera->SetTranslate({ 0.0f, 0.0f, -20.0f }); // 距離を調整
	camera->Update();

	//タイトルアニメーションの初期化
	animationTimer = 0.0f;
	for (size_t i = 0; i < kTitleCount; i++)
	{
		titleTranslates[i] = targetTranslates[i];
		titleTranslates[i].y = startY;
	}


	// --- 降ってくるコインの初期化 ---
	// 使用したいコインテクスチャのリスト
	const std::string coinTexturePaths[] = {
		"Resource/Coin(500).png",
		"Resource/Coin(100).png",
		"Resource/Coin(10).png",
		"Resource/Coin(-500).png"
	};
	const int coinTexCount = static_cast<int>(sizeof(coinTexturePaths) / sizeof(coinTexturePaths[0]));

	for (int i = 0; i < kCoinCount; i++)
	{
		coinObjects[i] = new Object3d();
		coinObjects[i]->Initialize();
		coinObjects[i]->SetModel("Coin.obj");

		// 4種類のテクスチャからランダムに1つ選んでインデックスを取得・設定
		int randTexIndex = std::rand() % coinTexCount;
		uint32_t coinTexIndex = TextureManager::GetInstance()->GetSrvIndex(coinTexturePaths[randTexIndex]);
		coinObjects[i]->SetTextureIndex(coinTexIndex);

		// X: -12〜12, Y: -8〜12 に散らす
		float randX = ((float)rand() / RAND_MAX) * 24.0f - 12.0f;
		float randY = ((float)rand() / RAND_MAX) * 20.0f - 8.0f;
		// Z: 2.0〜5.0（タイトル文字[Z=0]の奥、スカイボックスの手前）
		float randZ = ((float)rand() / RAND_MAX) * 3.0f + 2.0f;
		coinTranslates[i] = { randX, randY, randZ };

		// 初期回転角度（バラバラに傾ける）
		coinRotates[i] = {
			((float)rand() / RAND_MAX) * 6.28f,
			((float)rand() / RAND_MAX) * 6.28f,
			((float)rand() / RAND_MAX) * 6.28f
		};

		// スケール
		coinScales[i] = { 0.6f, 0.6f, 0.6f };

		// 落下速度と回転速度をセット
		coinFallSpeeds[i] = 0.05f + ((float)rand() / RAND_MAX) * 0.06f;
		coinRotSpeeds[i] = {
			0.01f + ((float)rand() / RAND_MAX) * 0.03f,
			0.02f + ((float)rand() / RAND_MAX) * 0.04f,
			0.01f + ((float)rand() / RAND_MAX) * 0.02f
		};
	}

	// --- 選択カーソル用コインの初期化 ---
	uint32_t selectCoinTexIndex = TextureManager::GetInstance()->GetSrvIndex("Resource/Coin(500).png");
	selectCoinObj = new Object3d();
	selectCoinObj->Initialize();
	selectCoinObj->SetModel("Coin.obj");
	selectCoinObj->SetTextureIndex(selectCoinTexIndex);

	// 初期位置を「START」の左側にセット
	selectCoinTranslate = { targetStartPos.x - 3.5f, targetStartPos.y, targetStartPos.z };
}

void TitleScene::Update()
{
	// ImGuiのフレーム開始処理
#ifdef USE_IMGUI
	ImGuiManager::GetInstance()->Begin();
	ImGuiManager::GetInstance()->UpdateTitleUI(spritePosition, spriteRotation, spriteSize, spriteColor, spriteSwitch,
		titleTranslates, titleRotates, titleScales,
		startTranslate, startRotate, startScale,
		endTranslate, endRotate, endScale,
		cameraTranslate, cameraRotate, skydomeSwitch
	);
#endif

	// パラメータの反映
	ParticleManager::GetInstance()->DrawImGui();

	sprite->SetPosition(spritePosition);
	sprite->SetRotation(spriteRotation);
	sprite->SetSize(spriteSize);
	sprite->SetColor(spriteColor);

	camera->SetTranslate(cameraTranslate);
	camera->SetRotate(cameraRotate);

	// 各種更新（行列計算など）
	skybox->Update(camera);
	sprite->Update();
	ParticleManager::GetInstance()->Update(camera);

	// タイマーを進める（60FPS想定）
	animationTimer += 1.0f / 60.0f;

	// パラメータ設定
	const float duration = 0.8f; // 1文字が落ちきるまでの時間（秒）
	const float delay = 0.24f;   // 次の文字が落ち始めるまでの時間差（秒）

	for (int i = 0; i < kTitleCount; i++)
	{
		// 文字ごとの開始時間
		float startTime = i * delay;

		// 進行度 (0.0f ～ 1.0f) の計算
		float t = std::clamp((animationTimer - startTime) / duration, 0.0f, 1.0f);

		// 加速落下（EaseInQuad）の計算
		float progress = Easing::EaseInQuad(t);

		// 落下開始位置（画面上の固定位置）
		Vector3 startPos = { targetTranslates[i].x, startY, targetTranslates[i].z };

		// Lerpで補間して座標をセット
		titleTranslates[i] = Easing::Lerp(startPos, targetTranslates[i], progress);

		if (titleObjects[i])
		{
			titleObjects[i]->SetTranslate(titleTranslates[i]);
			titleObjects[i]->SetRotate(titleRotates[i]);
			titleObjects[i]->SetScale(titleScales[i]);
			titleObjects[i]->Update();
		}
	}


	const float menuStartTime = 1.3f; // タイトル文字が落ち切った後（1.3秒）に開始
	const float menuDuration = 1.2f; // スッと登場するまでの時間（秒）

	if (startObj)
	{
		float t = std::clamp((animationTimer - menuStartTime) / menuDuration, 0.0f, 1.0f);
		float progress = Easing::EaseOutQuad(t);

		// 座標の補間（左画面外 -> 目標位置）
		Vector3 startBeginPos = { startOffscreenX, targetStartPos.y, targetStartPos.z };
		startTranslate = Easing::Lerp(startBeginPos, targetStartPos, progress);

		// Z軸回転（-720度[反時計回り2回転] -> 0度）
		startRotate.z = Easing::Lerp(-6.28f * 2.0f, 0.0f, progress);

		startObj->SetTranslate(startTranslate);
		startObj->SetRotate(startRotate);
		startObj->SetScale(startScale);
		startObj->Update();
	}

	if (endObj)
	{
		float t = std::clamp((animationTimer - (menuStartTime + 0.15f)) / menuDuration, 0.0f, 1.0f);
		float progress = Easing::EaseOutQuad(t);

		// 座標の補間（右画面外 -> 目標位置）
		Vector3 endBeginPos = { endOffscreenX, targetEndPos.y, targetEndPos.z };
		endTranslate = Easing::Lerp(endBeginPos, targetEndPos, progress);

		// Z軸回転（720度[時計回り2回転] -> 0度）
		endRotate.z = Easing::Lerp(6.28f * 2.0f, 0.0f, progress);

		endObj->SetTranslate(endTranslate);
		endObj->SetRotate(endRotate);
		endObj->SetScale(endScale);
		endObj->Update();
	}

	// 背景コインの落下・回転・ループ処理
	for (int i = 0; i < kCoinCount; i++)
	{
		if (coinObjects[i])
		{
			// 1. 下へ落下
			coinTranslates[i].y -= coinFallSpeeds[i];

			// 2. 各軸でクルクル回転
			coinRotates[i].x += coinRotSpeeds[i].x;
			coinRotates[i].y += coinRotSpeeds[i].y;
			coinRotates[i].z += coinRotSpeeds[i].z;

			// 3. 画面下（Y = -10.0f）まで落ちたら画面上（Y = 12.0f）に戻す
			if (coinTranslates[i].y < -10.0f)
			{
				coinTranslates[i].y = 12.0f;
				coinTranslates[i].x = ((float)rand() / RAND_MAX) * 24.0f - 12.0f;
			}

			// トランスフォーム反映と行列更新
			coinObjects[i]->SetTranslate(coinTranslates[i]);
			coinObjects[i]->SetRotate(coinRotates[i]);
			coinObjects[i]->SetScale(coinScales[i]);
			coinObjects[i]->Update();
		}
	}

	// -------------------------------------------------
	// 選択カーソル用コインの移動・回転制御
	// -------------------------------------------------
	// メニュー文字から左へ離す距離（モデルのサイズに合わせて -3.0f 〜 -4.0f 付近で調整）
	const float coinOffsetX = -3.5f;

	// 選択中のメニューの位置（スライドアニメーション中も追従するように startTranslate / endTranslate を参照）
	Vector3 targetCoinPos = { 0.0f, 0.0f, 0.0f };
	if (currentMenu == MenuType::kStart)
	{
		targetCoinPos = { startTranslate.x + coinOffsetX, startTranslate.y, startTranslate.z };
	}
	else if (currentMenu == MenuType::kEnd)
	{
		targetCoinPos = { endTranslate.x + coinOffsetX, endTranslate.y, endTranslate.z };
	}

	// 補間（Lerp）でぬるっと上下＆左右に移動させる
	selectCoinTranslate = Easing::Lerp(selectCoinTranslate, targetCoinPos, 0.25f);

	// X軸に90度（1.57f）傾けてコインの表面を正面（カメラ側）に向ける
	selectCoinRotate.x = 1.57f;

	// カーソルコインを自前でクルクル回転（Y軸回転）
	selectCoinRotate.y += 0.05f;

	if (selectCoinObj)
	{
		selectCoinObj->SetTranslate(selectCoinTranslate);
		selectCoinObj->SetRotate(selectCoinRotate);
		selectCoinObj->SetScale(selectCoinScale);
		selectCoinObj->Update();
	}

	// メニューが登場しきった後にキー入力受付を開始
	if (animationTimer >= menuStartTime + menuDuration)
	{
		// Wキー または 上矢印で「スタート」を選択
		if (Input::GetInstance()->TriggerKey(DIK_W) || Input::GetInstance()->TriggerKey(DIK_UP))
		{
			currentMenu = MenuType::kStart;
		}
		// Sキー または 下矢印で「エンド」を選択
		if (Input::GetInstance()->TriggerKey(DIK_S) || Input::GetInstance()->TriggerKey(DIK_DOWN))
		{
			currentMenu = MenuType::kEnd;
		}

		// ENTERキーでの決定処理
		if (Input::GetInstance()->TriggerKey(DIK_RETURN))
		{
#ifdef USE_IMGUI
			ImGuiManager::GetInstance()->End();
#endif
			if (currentMenu == MenuType::kStart)
			{
				// ゲームプレイシーンへ遷移
				SceneManager::GetInstance()->ChangeScene("GAMEPLAY");
				return;
			}
			else if (currentMenu == MenuType::kEnd)
			{
				// ゲーム終了（Win32メッセージを発行）
				PostQuitMessage(0);
				return;
			}
		}
	}
}

void TitleScene::Draw()
{
	// 描画先の変更・クリア（オフスクリーン描画）
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

	// SRVヒープのセット（オフスクリーン用）
	SrvManager::GetInstance()->PreDraw();

	//スプライトの描画
	SpriteCommon::GetInstance()->CommonDrawSettings();
	if (spriteSwitch)
	{
		D3D12_GPU_DESCRIPTOR_HANDLE titleTexHandle = TextureManager::GetInstance()->GetSrvHandleGPU("Resource/UI/title.png");
		sprite->Draw(DirectXCommon::GetInstance()->GetCommandList(), titleTexHandle);
	}

	//3Dオブジェクトの描画
	Object3dCommon::GetInstance()->CommonDrawSettings();
	for (int i = 0; i < kTitleCount; i++)
	{
		if(titleObjects[i])
		{
			titleObjects[i]->Draw();
		}
	}

	if (startObj)
	{
		startObj->Draw();
	}

	if (endObj)
	{
		endObj->Draw();
	}

	for (int i = 0; i < kCoinCount; i++)
	{
		if (coinObjects[i])
		{
			coinObjects[i]->Draw();
		}
	}

	// 選択カーソルコインの描画
	if (selectCoinObj)
	{
		selectCoinObj->Draw();
	}

	//スカイボックスの描画
	SkyboxCommon::GetInstance()->CommonDrawSettings(DirectXCommon::GetInstance()->GetCommandList());
	if (skydomeSwitch)
	{
		skybox->Draw();
	}
	

	// 5. バックバッファへ描画＆ポストプロセス
	renderTexture->ChangeState(DirectXCommon::GetInstance()->GetCommandList(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

	// バックバッファ準備
	DirectXCommon::GetInstance()->PreDraw();

	// ★【最重要】バックバッファ描画用にSRVディスクリプタヒープを再バインドする！
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

void TitleScene::Finalize()
{
	for (int i = 0; i < kTitleCount; i++)
	{
		delete titleObjects[i];
		titleObjects[i] = nullptr;
	}

	if (startObj)
	{
		delete startObj;
		startObj = nullptr;
	}

	if (endObj)
	{
		delete endObj;
		endObj = nullptr;
	}

	for (int i = 0; i < kCoinCount; i++)
	{
		if (coinObjects[i])
		{
			delete coinObjects[i];
			coinObjects[i] = nullptr;
		}
	}

	if (selectCoinObj)
	{
		delete selectCoinObj;
		selectCoinObj = nullptr;
	}

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