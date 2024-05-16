#include "pch.h"
#include "Hittable.h"
#include "Ray.h"

void hitData::setFaceNormal(const Ray& ray, const glm::dvec3& outwardNormal) {
	frontFace = glm::dot(ray.direction(), outwardNormal) < 0;
	normal = frontFace ? outwardNormal : -outwardNormal;
}