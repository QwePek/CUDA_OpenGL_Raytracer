#pragma once
#include "../Utils/Utils.h"
#include "../Utils/Interval.h"

class Ray
{
public:
	__device__ Ray(const glm::dvec3& origin, const glm::dvec3& direction) : orig(origin), dir(direction) {}

	__device__ const glm::dvec3& origin() const { return orig; }
	__device__ const glm::dvec3& direction() const { return dir; }

	__device__ glm::dvec3 at(double t) const {
		return orig + t * dir;
	}
private:
	glm::dvec3 orig;
	glm::dvec3 dir;
};