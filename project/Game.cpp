#include "Game.h"
#include "TextureManager.h"
#include "ModelManager.h"
#include "ParticleManager.h"
#include "SrvManager.h"
#include "SceneFactory.h"

void Game::Initialize()
{
	Framework::Initialize();

	// 基盤マネージャーの初期化
	TextureManager::GetInstance()->Initialize(DirectXCommon::GetInstance(), SrvManager::GetInstance());
	ModelManager::GetInstance()->Initialize(DirectXCommon::GetInstance());
	ParticleManager::GetInstance()->Initialize(DirectXCommon::GetInstance(),SrvManager::GetInstance());;

	// シーンマネージャの生成と初期化
	sceneFactory_ = new SceneFactory();
	SceneManager::GetInstance()->SetSceneFactory(sceneFactory_);

	SceneManager::GetInstance()->ChangeScene("GAMEPLAY");
}

void Game::Update()
{
	//基底クラスの更新処理
	Framework::Update();

	// シーンマネージャの更新処理
	SceneManager::GetInstance()->Update();
}

void Game::Draw()
{
	// シーンマネージャの描画処理
	SceneManager::GetInstance()->Draw();
}

void Game::Finalize()
{
	delete sceneFactory_;
	sceneFactory_ = nullptr;

	//各種マネジャーの終了処理
	TextureManager::GetInstance()->Finalize();
	ParticleManager::GetInstance()->Finalize();
	ModelManager::GetInstance()->Finalize();

	Framework::Finalize();
}