#pragma once
#include "../Utils/Utils.h"

#include "Hittable.h"

class Ray
{
public:
	Ray(const glm::vec3& origin, const glm::vec3& direction) : orig(origin), dir(direction) {}

	const glm::vec3& origin() const { return orig; }
	const glm::vec3& direction() const { return dir; }
	const glm::vec4 color(const Hittable& world) const {
		hitData data;
		if (world.hit(*this, Interval(0, Utils::infinity), data)) {
			return glm::vec4(0.5f * (data.normal + glm::vec3(1.0f, 1.0f, 1.0f)), 1.0f);
		}

		glm::vec3 unitDir = glm::normalize(dir);
		float a = 0.5f * (unitDir.y + 1.0f);
		//Gradient betweend white and (0.5f, 0.7f, 1.0f) color
		return glm::vec4((1.0f - a) * glm::vec3(1.0f, 1.0f, 1.0f) + a * glm::vec3(0.5f, 0.7f, 1.0f), 1.0f);
	}

	glm::vec3 at(float t) const {
		return orig + t * dir;
	}

private:
	glm::vec3 orig;
	glm::vec3 dir;
};