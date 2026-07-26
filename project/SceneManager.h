#pragma once
#include "BaseScene.h"
#include "AbstractSceneFactory.h"

//シーン管理
class SceneManager
{
public:
	static SceneManager* GetInstance();

	SceneManager(const SceneManager&) = delete;
	SceneManager& operator=(const SceneManager&) = delete;

	//今のシーン
	BaseScene* scene_ = nullptr;
	//次のシーン予約
	void SetNextScene(BaseScene* nextScene) { nextScene_ = nextScene; }
	
	void ChangeScene(const std::string& sceneName);

	//メンバ関数
	void Update();
	void Draw();
	void Finalize();

	//setter
	void SetSceneFactory(AbstractSceneFactory* sceneFactory) { sceneFactory_ = sceneFactory; }

private:
	SceneManager() = default;
	~SceneManager();
	
	//次のシーン
	BaseScene* nextScene_ = nullptr;
	//シーンファクトリー
	AbstractSceneFactory* sceneFactory_ = nullptr;
};

