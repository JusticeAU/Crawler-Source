#pragma once
#include "glm.hpp"
#include "gtx/easing.hpp"

class MathUtils
{
public:
	static float Lerp(float a, float b, float t)
	{
		return a * (1.0 - t) + (b * t);
	}
    static float EaseOutBounceSubtle(float t)
    {
        float easedT = glm::backEaseOut(t);
        if (easedT > 1.0f) easedT -= (easedT - 1.0f) * 0.85f;
        return easedT;
    }
	static glm::vec3 Lerp(glm::vec3 a, glm::vec3 b, float t)
	{
        glm::vec3 out;
		out.x = Lerp(a.x, b.x, t);
		out.y = Lerp(a.y, b.y, t);
		out.z = Lerp(a.z, b.z, t);
		return out;

	}

    static float LerpDegrees(float a, float b, float t)
    {
        float difference = glm::abs(a - b);
        if (difference > 180)
        {
            // We need to add on to one of the values.
            if (b > a)
            {
                // We'll add it on to start...
                a += 360;
            }
            else
            {
                // Add it on to end.
                b += 360;
            }
        }

        // Interpolate it.
        float value = (a + ((b - a) * t));

        // Wrap it..
        float rangeZero = 360;

        if (value >= 0 && value <= 360)
            return value;

        return fmod(value, rangeZero);
    }

	static float InverseLerp(float a, float b, float value)
	{
		return (value - a) / (b - a);
	}
};