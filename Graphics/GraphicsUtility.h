#pragma once
#define GL_GPU_MEM_INFO_TOTAL_AVAILABLE_MEM_NVX 0x9048
#define TEXTURE_FREE_MEMORY_ATI                 0x87FC
#include "glad.h"

#include "LogUtils.h"
#include <string>

using std::string;

namespace GraphicsUtility
{
	enum class Vendor
	{
		NVIDIA,
		AMD,
		UNKNOWN
	};
	string stringsNvidia[] =
	{
		"NVIDIA Corporation"
	};
	string stringsAMD[] =
	{
		"ATI Technologies Inc.",
		"AMD"
	};
	string stringsIntel[] =
	{
		"Intel"
	};

	Vendor GetVendorFromString(string vendorString);
	int GetVRAM(Vendor vendor);

	int GetVRAMTotal()
	{
		// get the vendor
		const GLubyte* vendorGL = glGetString(GL_VENDOR); // Returns the vendor
		string vendorString((char*)vendorGL);
		Vendor vendor = GetVendorFromString(vendorString);

		// get the vram total
		if (vendor != Vendor::UNKNOWN) return GetVRAM(vendor);
		else return -1;
	}

	Vendor GetVendorFromString(std::string vendorString)
	{
		// check for NVIDIA
		for (int i = 0; i < 1; i++)
		{
			if (vendorString == stringsNvidia[i]) return Vendor::NVIDIA;
		}

		// check for AMD
		for (int i = 0; i < 2; i++)
		{
			if (vendorString == stringsAMD[i]) return Vendor::AMD;
		}

		// Check for Intel ahaha
		return Vendor::UNKNOWN; // Just assume nvidia for now - Dat market share.
	}

	int GetVRAM(Vendor vendor)
	{
		GLenum location = GL_GPU_MEM_INFO_TOTAL_AVAILABLE_MEM_NVX;
		switch (vendor)
		{
		case Vendor::NVIDIA:
			location = GL_GPU_MEM_INFO_TOTAL_AVAILABLE_MEM_NVX;
			break;
		case Vendor::AMD:
			location = TEXTURE_FREE_MEMORY_ATI;
			break;
		}

		int total_memory_kb[4] = { 0,0,0,0 }; // The AMD variant returns 4 values, causes and overflow if you only allocate 1.
		glGetIntegerv(location, &total_memory_kb[0]);

		return total_memory_kb[0];
	}
}