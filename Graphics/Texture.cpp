#include "Texture.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
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
	int width, height, channels;
	/*stbi_info(name.c_str(), &width, &height, &channels);

	if (channels < 3)
	{
		LogUtils::Log("Error: Trying to load file with less than");
	}*/
	stbi_set_flip_vertically_on_load(true); // OpenGL expect y=0 to be the bottom of the texture.
	unsigned char* data = stbi_load(name.c_str(), &width, &height, &channels, 4);

	// transfer to VRAM
	glGenTextures(1, &texID);
	glBindTexture(GL_TEXTURE_2D, texID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

	// Configure this particular texture filtering
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_MAX_TEXTURE_MAX_ANISOTROPY, 16.0f); // need to do some tests on this one.

	// clean up
	glBindTexture(GL_TEXTURE_2D, 0);
	stbi_image_free(data);

	loaded = true;
}

void Texture::Bind(unsigned int slot)
{
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(GL_TEXTURE_2D, texID);
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
	name = "SSAO Noise Texture";
	loaded = true;
	references++;
	glGenTextures(1, &texID);
	glBindTexture(GL_TEXTURE_2D, texID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 4, 4, 0, GL_RGB, GL_FLOAT, noiseTexData);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
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
