#pragma once
#include "Raytracing/HittableList.h"
#include <curand_kernel.h>
#include "Raytracing/Materials/Material.h"

struct dataPixels {
	unsigned char r;
	unsigned char g;
	unsigned char b;
	unsigned char a;
};

__device__ inline double linearToGamma(double linearComponent) {
	if (linearComponent > 0)
		return sqrt(linearComponent);

	return 0;
}

class Camera
{
public:
	__device__ Camera(glm::vec3 _lookFrom, glm::vec3 _lookAt, glm::vec3 _vUp, float vFOV, float _focusDist, float _defocusAngle
		, float aspectRat, int imgWidth, int samplesPerPx = 10, int maxDepth = 10)
		: focusDistance(_focusDist), defocusAngle(_defocusAngle), verticalFov(vFOV), perPixelSamples(samplesPerPx),
		aspectRatio(aspectRat), lookFrom(_lookFrom), lookAt(_lookAt), vUp(_vUp), center(lookFrom), maxRecursionDepth(maxDepth)
	{
		pixelSampleScale = 1.0f / perPixelSamples;

		imageSize.x = imgWidth;
		imageSize.y = imageSize.x / aspectRatio;
		imageSize.y = (imageSize.y < 1) ? 1 : imageSize.y;

		float alpha = Utils::degToRad(verticalFov);
		float h = tan(alpha / 2.0f);
		glm::vec2 viewport(1.0f, 2 * h * focusDistance);
		viewport.x = viewport.y * (float(imageSize.x) / imageSize.y);

		//Basis vectors calculate
		w = glm::normalize(lookFrom - lookAt);
		u = glm::normalize(glm::cross(vUp, w));
		v = glm::cross(w, u);

		glm::vec3 viewport_u = viewport.x * u;
		glm::vec3 viewport_v = viewport.y * -v;

		pixelDelta_u = viewport_u / (float)imageSize.x;
		pixelDelta_v = viewport_v / (float)imageSize.y;

		glm::vec3 viewportUpperLeft = center - (focusDistance * w) - viewport_u / 2.0f - viewport_v / 2.0f;
		pixel00_loc = viewportUpperLeft + 0.5f * (pixelDelta_u + pixelDelta_v);

		//Calculate defocus disk vectors
		float defocusRadius = focusDistance * tan(Utils::degToRad(defocusAngle / 2.0));
		defocusDisk_u = u * defocusRadius;
		defocusDisk_v = v * defocusRadius;
	}

	__host__ __device__ inline glm::u32vec2 getImageSize() const { return imageSize; };
	__device__ float getPixelSampleScale() { return pixelSampleScale; };
	__device__ int getPerPixelSamples() { return perPixelSamples; };
	__device__ int getMaxRecursionDepth() { return maxRecursionDepth; };

	__device__ glm::vec3 rayColor(const Ray& ray, int depth, Hittable** world, curandState* localRandState) const
	{
		Ray cur_ray = ray;
		glm::vec3 cur_attenuation(1.0f, 1.0f, 1.0f);

		for (int i = 0; i < maxRecursionDepth; i++)
		{
			hitData rec;
			if ((*world)->hit(cur_ray, Interval(0.001f, Utils::infinity), rec))
			{
				Ray scattered(glm::vec3(0.0f), glm::vec3(0.0f));
				glm::vec3 attenuation;
				if (rec.mat->scatter(cur_ray, rec, attenuation, scattered, localRandState))
				{
					cur_attenuation *= attenuation;
					cur_ray = scattered;
				}
				else
					return glm::vec3(0.0, 0.0, 0.0);
			}
			else {
				glm::vec3 unitDir = glm::normalize(cur_ray.direction());

				float a = 0.5f * (unitDir.y + 1.0f);
				glm::vec3 c = (1.0f - a) * glm::vec3(1.0f, 1.0f, 1.0f) + a * glm::vec3(0.5f, 0.7f, 1.0f);
				return cur_attenuation * c;
			}
		}

		return glm::vec3(0.0f, 0.0f, 0.0f); // exceeded recursion


		//if (depth <= 0)
		//	return glm::vec3(0, 0, 0);

		//hitData data;
		//float x = Utils::infinity;
		//if ((*world)->hit(ray, Interval(0.001f, Utils::infinity), data)) {
		//	Ray scateredRay(glm::vec3(0.0f), glm::vec3(0.0f));
		//	glm::vec3 attenuation;
		//	if (data.mat->scatter(ray, data, attenuation, scateredRay, localRandState))
		//		return attenuation; //* rayColor(scateredRay, depth - 1, world, localRandState);
		//	return glm::vec3(0.0f, 0.0f, 0.0f);
		//}

		//glm::dvec3 unitDir = glm::normalize(ray.direction());

		//float a = 0.5f * (unitDir.y + 1.0f);
		////Gradient betweend white and (0.5f, 0.7f, 1.0f) color
		//return (1.0f - a) * glm::vec3(1.0f, 1.0f, 1.0f) + a * glm::vec3(0.5f, 0.7f, 1.0f);
	}


	__device__ Ray getRay(int i, int j, curandState* localRandState) const
	{
		glm::vec3 offset = sampleSquare(localRandState);
		glm::vec3 pixelCenter = pixel00_loc + (((float)i + offset.x) * pixelDelta_u)
			+ (((float)j + offset.y) * pixelDelta_v);

		glm::vec3 rayOrigin = (defocusAngle <= 0) ? center : sampleDefocusDisk(localRandState);
		glm::vec3 rayDir = pixelCenter - rayOrigin;

		return Ray(rayOrigin, rayDir);
	}

	__device__ dataPixels convertColor(const glm::vec3& color) {
		const Interval intensity(0.000, 0.999);

		glm::vec3 newColor = glm::vec3(linearToGamma(color.r), linearToGamma(color.g), linearToGamma(color.b));

		int r = int(256.0f * intensity.clamp(newColor.r));
		int g = int(256.0f * intensity.clamp(newColor.g));
		int b = int(256.0f * intensity.clamp(newColor.b));

		dataPixels ret = { r, g, b, 255 };
		return ret;
	}
private:
	//Camera helper functions
	__device__ glm::vec3 sampleSquare(curandState* localRandState) const {
		return glm::vec3(Utils::generateRandomNumber(-0.5, 0.5, localRandState),
			Utils::generateRandomNumber(-0.5, 0.5, localRandState), 0);
	}
	__device__ glm::vec3 sampleDefocusDisk(curandState* localRandState) const {
		glm::vec2 p = Utils::Vector::randomInUnitDisk(localRandState);
		return center + (p.x * defocusDisk_u) + (p.y * defocusDisk_v);
	}


	float pixelSampleScale;

	//Camera angles
	float verticalFov  = 90;
	glm::vec3 lookFrom	= glm::vec3(0.0, 0.0,  0.0);
	glm::vec3 lookAt	= glm::vec3(0.0, 0.0, -1.0);
	glm::vec3 vUp		= glm::vec3(0.0, 1.0,  0.0);
	glm::vec3 u, v, w; //Camera basis vectors

	//Defocus (blur :))
	float defocusAngle = 0;
	float focusDistance = 10;
	glm::vec3 defocusDisk_u; //defocus disk horizonal radius;
	glm::vec3 defocusDisk_v; //defocus disk vertical radius

	float aspectRatio = 1.0f;
	int perPixelSamples = 10;
	int maxRecursionDepth = 10;
	
	glm::u32vec2 imageSize = glm::u32vec2(100, 1);
	glm::vec3 center;
	glm::vec3 pixel00_loc;
	glm::vec3 pixelDelta_u;
	glm::vec3 pixelDelta_v;
};