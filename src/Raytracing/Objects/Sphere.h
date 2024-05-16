#pragma once
#include "../../Camera.h"

class Sphere : public Hittable
{
public:
	Sphere(const glm::vec3& center, double radius) : center(center), radius(radius < 0 ? 0 : radius) {}
	bool hit(const Ray& r, Interval rayT, hitData& data) const;

private:
	glm::vec3 center;
	double radius;
};