#pragma once
#include "../Utils/Utils.h"
#include "../Utils/Interval.h"

class Ray;
class hitData {
public:
	glm::vec3 p;
	glm::vec3 normal;
	double t;
	bool frontFace;

	//outwardNormal - unit length
	void setFaceNormal(const Ray& ray, const glm::vec3& outwardNormal);
};

class Hittable {
public:
	virtual ~Hittable() = default;

	virtual bool hit(const Ray& r, Interval rayT, hitData& data) const = 0;
};