#include "Game.h"
#include "TextureManager.h"
#include "ModelManager.h"
#include "ParticleManager.h"
#include "SrvManager.h"

void Game::Initialize()
{
	Framework::Initialize();

	// 基盤マネージャーの初期化
	TextureManager::GetInstance()->Initialize(DirectXCommon::GetInstance(), SrvManager::GetInstance());
	ModelManager::GetInstance()->Initialize(DirectXCommon::GetInstance());
	ParticleManager::GetInstance()->Initialize(DirectXCommon::GetInstance(),SrvManager::GetInstance());

	//ゲームプレイシーンの生成
	scene_ = new GamePlayScene();
	//ゲームプレイシーンの初期化
	scene_->Initialize();
}

void Game::Update()
{
	//基底クラスの更新処理
	Framework::Update();

	//シーンの更新処理
	scene_->Update();
}

void Game::Draw()
{
	//シーン描画
	scene_->Draw();
}

void Game::Finalize()
{
	//シーンの終了処理
	scene_->Finalize();
	//シーンの解放
	delete scene_;
	scene_ = nullptr;

	//各種マネジャーの終了処理
	TextureManager::GetInstance()->Finalize();
	ParticleManager::GetInstance()->Finalize();
	ModelManager::GetInstance()->Finalize();

	Framework::Finalize();
}