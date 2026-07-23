#pragma once

#include "SpriteCommon.h"
#include "Sprite.h"
#include "TextureManager.h"
#include "Object3dCommon.h"
#include "Object3d.h"
#include "ModelManager.h"
#include "Model.h"
#include "Camera.h"
#include "ParticleManager.h"
#include "Skybox.h"
#include "SkyboxCommon.h"
#include "RenderTexture.h"
#include "PostProcess.h"


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

