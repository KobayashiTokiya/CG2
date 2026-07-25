#pragma once

#include "Framework.h"
#include "GamePlayScene.h"

class Game:public Framework
{
public:
	void Initialize()override;
	void Finalize()override;
	void Update()override;
	void Draw()override;

private:
	//シーン
	GamePlayScene* scene_ = nullptr;
};

