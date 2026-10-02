#include "SceneManager.h"
#include "DirectXCommon.h"
#include <cassert>
SceneManager* SceneManager::GetInstance()
{
	static SceneManager instance;
	return &instance;
}

void SceneManager::Update()
{
	// 次シーンの予約があるなら
	if (nextScene_)
	{
		// 旧シーンの終了
		if (scene_)
		{
			// ★ GPUの描画が完了するまで待つ（これでDevice Removedを防ぐ）
			DirectXCommon::GetInstance()->WaitForGpu();

			scene_->Finalize();
			delete scene_;
		}

		// シーン切り換え
		scene_ = nextScene_;
		nextScene_ = nullptr;

		scene_->SetSceneManager(this);
		scene_->Initialize();
	}

	scene_->Update();
}

void SceneManager::Draw()
{
	scene_->Draw();
}

SceneManager::~SceneManager()
{
	if (scene_)
	{
		scene_->Finalize();
		delete scene_;
		scene_ = nullptr;
	}
}

void SceneManager::ChangeScene(const std::string& sceneName)
{
	assert(sceneFactory_);
	assert(nextScene_ == nullptr);

	nextScene_ = sceneFactory_->CreateScene(sceneName);
}