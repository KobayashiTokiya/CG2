#include "Sprite.h"
#include "SpriteCommon.h"

void Sprite::Initialize(SpriteCommon* spriteCommon)
{
	//引数で受け取ってメンバ変数に記録する
	this->spriteCommon = spriteCommon;
}