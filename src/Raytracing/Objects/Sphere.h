#pragma once
#include "../../Camera.h"

class Sphere : public Hittable
{
public:
	Sphere(const glm::dvec3& center, double radius, std::shared_ptr<Material> mat) : 
		center(center), radius(radius < 0 ? 0 : radius), mat(mat) {}
	bool hit(const Ray& r, Interval rayT, hitData& data) const;

private:
	glm::dvec3 center;
	double radius;
	std::shared_ptr<Material> mat;
};