#pragma once
#include "Vector.h"

class CollisionManager
{
public:
	// 単純な3D球体判定
	static bool CheckSphere(const Vector3& posA, float radiusA, const Vector3& posB, float radiusB);

	// コップ（円柱）の底に入ったかどうかの判定
	static bool CheckCupBottom(const Vector3& cupPos, const Vector3& coinPos, float cupHeight, float innerRadius);

	// コップ（円柱）のフチ（壁）に当たったかどうかの判定
	static bool CheckCupWall(const Vector3& cupPos, const Vector3& coinPos, float cupHeight, float innerRadius, float wallThickness);

	static bool CheckCupWallAndGetBounce(
		const Vector3& cupPos, const Vector3& coinPos,
		float cupHeight, float innerRadius, float wallThickness,
		Vector3& outBounceDir, Vector3& outPushPos);

};

