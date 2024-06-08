#pragma once
#include "Ray.h"

class Material;
class hitData {
public:
	glm::vec3 p;
	glm::vec3 normal;
	Material* mat;
	float t;
	bool frontFace;

	//outwardNormal - unit length
	__device__ void setFaceNormal(const Ray& ray, const glm::vec3& outwardNormal) {
		frontFace = glm::dot(ray.direction(), outwardNormal) < 0;
		normal = frontFace ? outwardNormal : -outwardNormal;
	}
};

class Hittable {
public:
	__device__ virtual ~Hittable() = default;

	__device__ virtual bool hit(const Ray& r, Interval rayT, hitData& data) const = 0;
};