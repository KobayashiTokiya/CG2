#include "SceneManager.h"
#include <cassert>
SceneManager* SceneManager::GetInstance()
{
	static SceneManager instance;
	return &instance;
}

void SceneManager::Update()
{
	//次シーンの予約があるなら
	if (nextScene_)
	{
		//旧シーンの終了
		if (scene_)
		{
			scene_->Finalize();
			delete scene_;
		}

		//シーン切り換え
		scene_ = nextScene_;
		nextScene_ = nullptr;
		
		//シーンマネージャをセット
		scene_->SetSceneManager(this);

		//次シーンを初期化する
		scene_->Initialize();
	}

	//実行中シーンを更新する
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