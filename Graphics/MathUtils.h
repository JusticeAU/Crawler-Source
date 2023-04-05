#pragma once

class MathUtils
{
public:
	static float Lerp(float a, float b, float t)
	{
		return a * (1.0 - t) + (b * t);
	}
};