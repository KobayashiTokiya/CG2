#include "Easing.h"


float Easing::Lerp(float start, float end, float t)
{
	return start + (end - start) * t;
}

Vector3 Easing::Lerp(const Vector3& start, const Vector3& end, float t)
{
	return {
		Lerp(start.x, end.x, t),
		Lerp(start.y, end.y, t),
		Lerp(start.z, end.z, t)
	};
}

float Easing::EaseInQuad(float t)
{
	return t * t;
}

float Easing::EaseOutQuad(float t)
{
	return 1.0f - (1.0f - t) * (1.0f - t);
}

float Easing::EaseInOutQuad(float t)
{
	return t < 0.5f ? 2.0f * t * t : 1.0f - std::pow(-2.0f * t + 2.0f, 2.0f) / 2.0f;
}

float Easing::EaseOutBounce(float t)
{
	const float n1 = 7.5625f;
	const float d1 = 2.75f;

	if (t < 1.0f / d1)
	{
		return n1 * t * t;
	}
	else if (t < 2.0f / d1)
	{
		t -= 1.5f / d1;
		return n1 * t * t + 0.75f;
	}
	else if (t < 2.5f / d1)
	{
		t -= 2.25f / d1;
		return n1 * t * t + 0.9375f;
	}
	else
	{
		t -= 2.625f / d1;
		return n1 * t * t + 0.984375f;
	}
}