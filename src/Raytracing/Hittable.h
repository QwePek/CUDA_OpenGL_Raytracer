#pragma once
#include "Ray.h"

class Material;
class hitData {
public:
	__device__ ~hitData() { delete mat; }

	glm::dvec3 p;
	glm::dvec3 normal;
	Material* mat;
	double t;
	bool frontFace;

	//outwardNormal - unit length
	__device__ void setFaceNormal(const Ray& ray, const glm::dvec3& outwardNormal) {
		frontFace = glm::dot(ray.direction(), outwardNormal) < 0;
		normal = frontFace ? outwardNormal : -outwardNormal;
	}
};

class Hittable {
public:
	__device__ virtual ~Hittable() = default;

	__device__ virtual bool hit(const Ray& r, Interval rayT, hitData& data) const = 0;
};