#pragma once
#include "glm.hpp"

class MathUtils
{
public:
	static float Lerp(float a, float b, float t)
	{
		return a * (1.0 - t) + (b * t);
	}
	static vec3 Lerp(vec3 a, vec3 b, float t)
	{
		vec3 out;
		out.x = Lerp(a.x, b.x, t);
		out.y = Lerp(a.y, b.y, t);
		out.z = Lerp(a.z, b.z, t);
		return out;

	}
	static float InverseLerp(float a, float b, float value)
	{
		return (value - a) / (b - a);
	}
};