#pragma once
#include "../Utils/Utils.h"
#include "../Utils/Interval.h"

class Ray;
class Material;
class hitData {
public:
	glm::dvec3 p;
	glm::dvec3 normal;
	std::shared_ptr<Material> mat;
	double t;
	bool frontFace;

	//outwardNormal - unit length
	void setFaceNormal(const Ray& ray, const glm::dvec3& outwardNormal);
};

class Hittable {
public:
	virtual ~Hittable() = default;

	virtual bool hit(const Ray& r, Interval rayT, hitData& data) const = 0;
};