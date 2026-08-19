#include "CollisionManager.h"
#include <cmath>

bool CollisionManager::CheckSphere(const Vector3& posA, float radiusA, const Vector3& posB, float radiusB)
{
	Vector3 diff = { posA.x - posB.x, posA.y - posB.y, posA.z - posB.z };
	float distanceSq = diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;
	float radiusSum = radiusA + radiusB;
	return distanceSq <= (radiusSum * radiusSum);
}

bool CollisionManager::CheckCupBottom(const Vector3& cupPos, const Vector3& coinPos, float cupHeight, float innerRadius)
{
	// 高さの判定（底付近にいるか）
	if (coinPos.y < cupPos.y || coinPos.y > cupPos.y + 0.3f)
	{
		return false;
	}

	// XZ平面での距離計算
	float dx = coinPos.x - cupPos.x;
	float dz = coinPos.z - cupPos.z;
	float distanceSq = dx * dx + dz * dz;

	return distanceSq <= (innerRadius * innerRadius);
}

bool CollisionManager::CheckCupWall(const Vector3& cupPos, const Vector3& coinPos, float cupHeight, float innerRadius, float wallThickness)
{
	// 高さの判定（コップの側面の高さ範囲内にあるか）
	if (coinPos.y < cupPos.y || coinPos.y > cupPos.y + cupHeight)
	{
		return false;
	}

	// XZ平面での距離計算
	float dx = coinPos.x - cupPos.x;
	float dz = coinPos.z - cupPos.z;
	float distanceSq = dx * dx + dz * dz;

	float outerRadius = innerRadius + wallThickness;

	// 内径と外径の間にコインがあれば「壁にヒット」
	return (distanceSq >= innerRadius * innerRadius) && (distanceSq <= outerRadius * outerRadius);
}

bool CollisionManager::CheckCupWallAndGetBounce(
    const Vector3& cupPos, const Vector3& coinPos,
    float cupHeight, float innerRadius, float wallThickness,
    Vector3& outBounceDir, Vector3& outPushPos)
{
    // 高さの判定（コップの側面の高さ範囲内にあるか）
    if (coinPos.y < cupPos.y || coinPos.y > cupPos.y + cupHeight)
    {
        return false;
    }

    float dx = coinPos.x - cupPos.x;
    float dz = coinPos.z - cupPos.z;
    float distanceSq = dx * dx + dz * dz;

    float outerRadius = innerRadius + wallThickness;

    // 内径と外径の間にコインがあれば「壁にヒット」
    if (distanceSq >= (innerRadius * innerRadius) && distanceSq <= (outerRadius * outerRadius))
    {
        float distXZ = std::sqrt(distanceSq);

        // 弾く方向（コップの中心から外側へ）
        if (distXZ > 0.001f)
        {
            outBounceDir.x = dx / distXZ;
            outBounceDir.y = 0.0f;
            outBounceDir.z = dz / distXZ;
        }
        else
        {
            outBounceDir = { 1.0f, 0.0f, 0.0f }; // 真上などで計算できない場合の予備
        }

        // 壁の外側に押し出す位置を計算（貫通防止）
        outPushPos = coinPos;
        outPushPos.x = cupPos.x + outBounceDir.x * (outerRadius + 0.1f);
        outPushPos.z = cupPos.z + outBounceDir.z * (outerRadius + 0.1f);

        return true;
    }

    return false;
}