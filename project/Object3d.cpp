#include "Object3d.h"
#include "Object3dCommon.h"

void Object3d::Initialize(Object3dCommon* object3dCommon)
{
	//引数で受け取ってメンバ変数に記録する
	this->object3dCommon = object3dCommon;

	//モデル読み込み
	modelData = LoadObjFile("Resource", "plane.obj");
}

void Object3d::Update()
{

}

void Object3d::Draw()
{

}