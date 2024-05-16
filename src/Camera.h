#pragma once
#include "Raytracing/HittableList.h"

struct dataPixels {
	unsigned char r;
	unsigned char g;
	unsigned char b;
	unsigned char a;
};

class Camera
{
public:
	Camera(double aspectRat, int imgWidth, int samplesPerPx = 10, int maxDepth = 10);
	void render(const Hittable& world);

	inline glm::u32vec2 getImageSize() const { return imageSize; };
	const std::vector<dataPixels>& getPixelData() { return data; };

private:
	glm::vec3 rayColor(const Ray& ray, int depth, const Hittable& world) const;
	Ray getRay(int i, int j) const;

	//Camera helper functions
	glm::vec3 sampleSquare() const;
	dataPixels convertColor(const glm::vec3& color);

	double pixelSampleScale;
	double aspectRatio = 1.0f;
	int perPixelSamples = 10;
	int maxRecursionDepth = 10;
	
	glm::u32vec2 imageSize = glm::u32vec2(100, 1);
	glm::vec3 center;
	glm::vec3 pixel00_loc;
	glm::vec3 pixelDelta_u;
	glm::vec3 pixelDelta_v;

	std::vector<dataPixels> data;
};