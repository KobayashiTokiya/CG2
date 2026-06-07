#pragma once
#include "Vector.h"

struct AABB
{
	Vector3 min;//最小点
	Vector3 max;//最大点
};

class Collision
{
public:
	static bool IsCollision(const AABB& aabb, const Vector3& point);
};

