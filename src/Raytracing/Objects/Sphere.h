#pragma once
#include "../../Utils/Utils.h"
#include "../Ray.h"

class Sphere : public Hittable
{
public:
	Sphere(const glm::vec3& center, double radius) : center(center), radius(radius < 0 ? 0 : radius) {}

	bool hit(const Ray& r, Interval rayT, hitData& data) const {
		//oc = C - Q
		glm::vec3 oc = center - r.origin();
		//elementy rownania kwadratowego
		double a = glm::dot(r.direction(), r.direction()); //To to samo co dlugosc^2
		double h = glm::dot(r.direction(), oc); //d = dir
		double c = glm::dot(oc, oc) - radius * radius;

		double delta = h * h - a * c;
		if (delta < 0)
			return false;

		double deltaSqr = sqrt(delta);
		//Nearest solution that lies within range
		float root = (h - deltaSqr) / a;
		if (!rayT.surrounds(root)) {
			root = (h + deltaSqr) / a;
			if (!rayT.surrounds(root))
				return false;
		}

		data.t = root;
		data.p = r.at(root);
		glm::vec3 outwardNormal = (data.p - center) / (float)radius;
		data.setFaceNormal(r, outwardNormal);

		return true;
	}

private:
	glm::vec3 center;
	double radius;
};