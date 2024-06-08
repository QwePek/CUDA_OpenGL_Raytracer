#pragma once
#include "../Utils/Utils.h"
#include "../Utils/Interval.h"

class Ray
{
public:
	__device__ Ray(const glm::vec3& origin, const glm::vec3& direction) : orig(origin), dir(direction) {}

	__device__ const glm::vec3& origin() const { return orig; }
	__device__ const glm::vec3& direction() const { return dir; }

	__device__ glm::vec3 at(float t) const {
		return orig + t * dir;
	}
private:
	glm::vec3 orig;
	glm::vec3 dir;
};