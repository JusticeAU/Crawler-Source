#include "GraphicsQuality.h"
#include "GraphicsUtility.h"

GraphicsQuality::Quality GraphicsQuality::m_quality = Quality::Auto;

void GraphicsQuality::SetQuality(Quality quality)
{
	if (quality == Quality::Auto)
	{

		int vram = GraphicsUtility::GetVRAMTotal();
		if (vram != -1)
		{
			if (vram > (vramHigh * 1024))
				m_quality = Quality::High;
			if (vram <= (vramHigh * 1024))
				m_quality = Quality::Medium;
			if (vram < (vramMedium * 1024))
				m_quality = Quality::Low;

		}
		else m_quality = Quality::High;
	}
	else
	{
		m_quality = quality;
	}

	switch (m_quality)
	{
	case Quality::High:
		LogUtils::Log("Setting Graphics Quality to High.");
		break;
	case Quality::Medium:
		LogUtils::Log("Setting Graphics Quality to Medium.");
		break;
	case Quality::Low:
		LogUtils::Log("Setting Graphics Quality to Low.");
		break;
	}
}
