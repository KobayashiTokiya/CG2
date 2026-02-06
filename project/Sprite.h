#pragma once
#include "main.cpp"

class  SpriteCommon;

//頂点データ
struct VertexData
{
	Vector4 position;
	Vector2 texcoord;
	Vector3 normal;
};

//スプライト
class Sprite
{
public:// メンバ関数
	//初期化
	void Initialize(SpriteCommon* spriteCommon);

private:
	SpriteCommon* spriteCommon = nullptr;
};

