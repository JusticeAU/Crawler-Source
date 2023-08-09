#include "Texture.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

Texture::Texture() : texID(0)
{
}

Texture::~Texture()
{
	glDeleteTextures(1, &texID);
}

void Texture::LoadFromFile(string filename)
{
	// Load with stb_image
	int width, height, channels;
	stbi_set_flip_vertically_on_load(true); // OpenGL expect y=0 to be the bottom of the texture.
	unsigned char* data = stbi_load(filename.c_str(), &width, &height, &channels, 3);

	// transfer to VRAM
	glGenTextures(1, &texID);
	glBindTexture(GL_TEXTURE_2D, texID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

	// Configure this particular texture filtering
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// clean up
	glBindTexture(GL_TEXTURE_2D, 0);
	stbi_image_free(data);
	name = filename;
}

void Texture::Bind(unsigned int slot)
{
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(GL_TEXTURE_2D, texID);
}
