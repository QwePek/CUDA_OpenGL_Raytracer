#pragma once
#include <iostream>

class Texture
{
public:
	Texture(const std::string& path);
	Texture(unsigned char* textureBuffer, int32_t _width, int32_t _height);
	~Texture();

	void bind(uint32_t slot = 0) const;
	void unbind() const;

	void updateData(unsigned char* textureBuffer);

	inline int getWidth() const { return width; }
	inline int getHeight() const { return height; }

	inline uint32_t getTextureID() { return texID; }

private:
	uint32_t texID;
	std::string texturePath;

	unsigned char* textureBuffer;
	int width, height, numOfChannels;
};