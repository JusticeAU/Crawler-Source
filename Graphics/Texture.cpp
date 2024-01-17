#include "Texture.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#define STB_IMAGE_RESIZE2_IMPLEMENTATION
#include "stb_image_resize2.h";
#include "Window.h"
#include "LogUtils.h"

Texture::Texture() : texID(0)
{
}

Texture::Texture(string filename)
{
	name = filename;
}

Texture::~Texture()
{
	glDeleteTextures(1, &texID);
}

void Texture::LoadFromFile(string filename)
{
	name = filename;
	Load();
}

void Texture::Load()
{
	LogUtils::Log("Loading: " + name);

	if(loaded)
		glDeleteTextures(1, &texID);

	// Load with stb_image
	stbi_set_flip_vertically_on_load(true); // OpenGL expect y=0 to be the bottom of the texture.
	int requestedChannels = channels != -1 ? channels : 4;
	unsigned char* data = stbi_load(name.c_str(), &width, &height, &channels, requestedChannels);
	unsigned char* dataProcessed = nullptr;
	bool resized = false;
	switch (quality)
	{
	case Quality::Low:
	{
		resized = true;
		int newWidth, newHeight;
		int layout = GetLayoutFromChannels(requestedChannels);
		newWidth = glm::max(1,(int)(width * m_qualityScales[(int)quality]));
		newHeight = glm::max(1,(int)(height * m_qualityScales[(int)quality]));
		dataProcessed = stbir_resize_uint8_linear(data, width, height, 0, 0, newWidth, newHeight, 0, (stbir_pixel_layout)layout);
		width = newWidth;
		height = newHeight;
		break;
	}
	case Quality::Medium:
	{
		resized = true;
		int newWidth, newHeight;
		int layout = GetLayoutFromChannels(requestedChannels);
		newWidth = (float)width * m_qualityScales[(int)quality];
		newHeight = (float)height * m_qualityScales[(int)quality];
		dataProcessed = stbir_resize_uint8_linear(data, width, height, 0, 0, newWidth, newHeight, 0, (stbir_pixel_layout)layout);
		width = newWidth;
		height = newHeight;
		break;
	}
	case Quality::High:
	{
		dataProcessed = data;
		break;
	}
	}
	
	// Modifcations have been made to stbi__convert_format in order to have it load images to a single channel the way I want.
	// Out of the box is takes the RGB values and calculate luminosity and returns that, but I just want the red channel only.
	GLint internalType;
	switch (requestedChannels)
	{
	case 1:
		internalType = GL_RED;
		break;
	case 2:
		internalType = GL_RG;
		break;
	case 3:
		internalType = GL_RGB;
		break;
	default:
		internalType = GL_RGBA;
		break;
	}

	// transfer to VRAM
	glGenTextures(1, &texID);
	glBindTexture(GL_TEXTURE_2D, texID);
	glTexImage2D(GL_TEXTURE_2D, 0, internalType, width, height, 0, internalType, GL_UNSIGNED_BYTE, dataProcessed);

	// Configure this particular texture filtering
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
	// Enable Anisotropic Filtering
	GLfloat value, max_anisotropy = 8.0f; /* don't exceed this value...*/
	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &value);
	value = (value > max_anisotropy) ? max_anisotropy : value;
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY , value); // need to do some tests on this one.

	// clean up
	glBindTexture(GL_TEXTURE_2D, 0);
	stbi_image_free(data);
	if(resized) stbi_image_free(dataProcessed);

	loaded = true;
}

void Texture::Resize()
{
}

int Texture::GetLayoutFromChannels(int channels)
{
	switch (channels)
	{
	case 1:
		return STBIR_1CHANNEL;
	case 2:
		return STBIR_2CHANNEL;
	case 3:
		return STBIR_RGB;
	case 4:
		return STBIR_RGBA;
	}
}

void Texture::Bind(unsigned int slot)
{
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(texType, texID);
}

//void Texture::CreateSSAOColourBuffer()
//{
//	name = "SSAO Colour Buffer";
//	glm::ivec2 size = Window::GetViewPortSize();
//	glGenTextures(1, &texID);
//	glBindTexture(GL_TEXTURE_2D, texID);
//	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, size.x, size.y, 0, GL_RGBA, GL_FLOAT, NULL);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//}
//
//void Texture::ResizeSSAColourBuffer()
//{
//	glDeleteTextures(1, &texID);
//	CreateSSAOColourBuffer();
//}

void Texture::CreateSSAONoiseTexture(glm::vec3* noiseTexData)
{
	name = "ssao_noise_texture";
	loaded = true;
	width = 4;
	height = 4;
	glGenTextures(1, &texID);
	glBindTexture(GL_TEXTURE_2D, texID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGB, GL_FLOAT, noiseTexData);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

void Texture::CreateRandomTexture(unsigned int size)
{
	glm::vec3* pRandomData = new glm::vec3[size];

	for (unsigned int i = 0; i < size; i++) {
		pRandomData[i].x = (float)rand() / RAND_MAX;
		pRandomData[i].y = (float)rand() / RAND_MAX;
		pRandomData[i].z = (float)rand() / RAND_MAX;
	}

	glGenTextures(1, &texID);
	glBindTexture(GL_TEXTURE_1D, texID);
	glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB, size, 0.0f, GL_RGB, GL_FLOAT, pRandomData);
	glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);

	delete[] pRandomData;
	name = "random_texture";
	texType = GL_TEXTURE_1D;
	loaded = true;
}

void Texture::RewriteTGAwithRLE(string from, string to)
{
	// Load with stb_image
	stbi_set_flip_vertically_on_load(false);
	int width, height, channels;
	unsigned char* data = stbi_load(from.c_str(), &width, &height, &channels, 0); // load with auto components to ensure we're not destructive to the alpha or whatever.
	// Write it back out. Easy Peasy.	
	stbi_write_tga(to.c_str(), width, height, channels, data);
	stbi_image_free(data);
}
