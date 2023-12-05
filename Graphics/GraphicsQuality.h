#pragma once

class GraphicsQuality
{
public:
	enum class Quality
	{
		Low,
		Medium,
		High,
		Auto
	};

	static Quality m_quality;
	static const unsigned int vramHigh = 6000;
	static const unsigned int vramMedium = 2400;

	static void DetectQuality();
};