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
    if (coinPos.y < cupPos.y - 0.5f || coinPos.y > cupPos.y + cupHeight + 1.0f)
    {
        return false;
    }

    // XZ平面での距離計算（影と重なっているか）
    float dx = coinPos.x - cupPos.x;
    float dz = coinPos.z - cupPos.z;
    float distanceSq = dx * dx + dz * dz;

    // コップの内径に入っていれば即キャッチ成立
    return distanceSq <= (innerRadius * innerRadius);
}

bool CollisionManager::CheckCupWallAndGetBounce(
    const Vector3& cupPos, const Vector3& coinPos,
    float cupHeight, float innerRadius, float wallThickness,
    Vector3& outBounceDir, Vector3& outPushPos)
{
    // ★修正: コップの口（上端から0.4fの範囲）を通過しているときは壁として扱わない（上からの進入を許可）
    float openTopY = cupPos.y + cupHeight - 0.4f;
    if (coinPos.y >= openTopY)
    {
        return false;
    }

    // 高さの判定（側面部分のみ）
    if (coinPos.y < cupPos.y || coinPos.y > cupPos.y + cupHeight)
    {
        return false;
    }

    float dx = coinPos.x - cupPos.x;
    float dz = coinPos.z - cupPos.z;
    float distanceSq = dx * dx + dz * dz;

    float outerRadius = innerRadius + wallThickness;

    // 内径と外径の間にコインがあれば「壁（側面）にヒット」
    if (distanceSq >= (innerRadius * innerRadius) && distanceSq <= (outerRadius * outerRadius))
    {
        float distXZ = std::sqrt(distanceSq);

        if (distXZ > 0.001f)
        {
            outBounceDir.x = dx / distXZ;
            outBounceDir.y = 0.0f;
            outBounceDir.z = dz / distXZ;
        }
        else
        {
            outBounceDir = { 1.0f, 0.0f, 0.0f };
        }

        outPushPos = coinPos;
        outPushPos.x = cupPos.x + outBounceDir.x * (outerRadius + 0.1f);
        outPushPos.z = cupPos.z + outBounceDir.z * (outerRadius + 0.1f);

        return true;
    }

    return false;
}