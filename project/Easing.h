#pragma once
#include "Vector.h"

class Easing
{
public:
	static float Lerp(float start, float end, float t);

	static Vector3 Lerp(const Vector3& start, const Vector3& end, float t);

	static float EaseInQuad(float t);
	static float EaseOutQuad(float t);
	static float EaseInOutQuad(float t);
	static float EaseOutBounce(float t);
};

