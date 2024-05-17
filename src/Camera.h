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
	Camera(glm::dvec3 _lookFrom, glm::dvec3 _lookAt, glm::dvec3 _vUp, double vFOV, double _focusDist, double _defocusAngle
		, double aspectRat, int imgWidth, int samplesPerPx = 10, int maxDepth = 10);
	void render(const Hittable& world);

	inline glm::u32vec2 getImageSize() const { return imageSize; };
	const std::vector<dataPixels>& getPixelData() { return data; };

private:
	glm::dvec3 rayColor(const Ray& ray, int depth, const Hittable& world) const;
	Ray getRay(int i, int j) const;

	//Camera helper functions
	glm::dvec3 sampleSquare() const;
	glm::dvec3 sampleDefocusDisk() const;
	dataPixels convertColor(const glm::dvec3& color);

	double pixelSampleScale;

	//Camera angles
	double verticalFov  = 90;
	glm::dvec3 lookFrom = glm::dvec3(0.0, 0.0,  0.0);
	glm::dvec3 lookAt   = glm::dvec3(0.0, 0.0, -1.0);
	glm::dvec3 vUp		= glm::dvec3(0.0, 1.0,  0.0);
	glm::dvec3 u, v, w; //Camera basis vectors

	//Defocus (blur :))
	double defocusAngle = 0;
	double focusDistance = 10;
	glm::dvec3 defocusDisk_u; //defocus disk horizonal radius;
	glm::dvec3 defocusDisk_v; //defocus disk vertical radius

	double aspectRatio = 1.0f;
	int perPixelSamples = 10;
	int maxRecursionDepth = 10;
	
	glm::u32vec2 imageSize = glm::u32vec2(100, 1);
	glm::dvec3 center;
	glm::dvec3 pixel00_loc;
	glm::dvec3 pixelDelta_u;
	glm::dvec3 pixelDelta_v;

	std::vector<dataPixels> data;
};