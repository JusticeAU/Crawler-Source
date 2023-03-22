#include "Texture.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

void Texture::LoadFromFile(string filename)
{
	// Load with stb_image
	int width, height, channels;
	unsigned char* data = stbi_load(filename.c_str(), &width, &height, &channels, 3);

	// transfer to VRAM
	glGenTextures(1, &texID);
	glBindTexture(GL_TEXTURE_2D, texID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

	// Configure this particular texture filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// clean up
	glBindTexture(GL_TEXTURE_2D, 0);
	stbi_image_free(data);
}

void Texture::Bind(unsigned int slot)
{
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(GL_TEXTURE_2D, texID);
}
