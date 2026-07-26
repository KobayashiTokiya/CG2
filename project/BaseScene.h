#pragma once
class SceneManager;

//シーン基底クラス
class BaseScene
{
public:
	virtual ~BaseScene() = default;

	virtual void Initialize()=0;
	virtual void Finalize()=0;
	virtual void Update()=0;
	virtual void Draw()=0;

	//setter
	virtual void SetSceneManager(SceneManager* sceneManager) { sceneManager_ = sceneManager; }

	//getter
	SceneManager* GetSceneManager()const { return sceneManager_; }

private:
	//シーンマネージャー
	SceneManager* sceneManager_ = nullptr;
};

