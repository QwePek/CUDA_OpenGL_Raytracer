#include "pch.h"
#include "Hittable.h"
#include "Ray.h"
#include "glm\glm.hpp"
#include "glm\gtc\matrix_transform.hpp"
#include "glm\gtc\type_ptr.hpp"


void hitData::setFaceNormal(const Ray& ray, const glm::vec3& outwardNormal) {
	frontFace = glm::dot(ray.direction(), outwardNormal) < 0;
	normal = frontFace ? outwardNormal : -outwardNormal;
}