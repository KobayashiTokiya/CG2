#pragma once
#include "Framework.h"
#include "SceneManager.h"
#include "AbstractSceneFactory.h"

class Game:public Framework
{
public:
	void Initialize()override;
	void Finalize()override;
	void Update()override;
	void Draw()override;

private:
	//シーンファクトリー
	AbstractSceneFactory* sceneFactory_ = nullptr;
};

