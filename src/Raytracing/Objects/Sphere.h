#pragma once
#include "../../Camera.h"

class Sphere : public Hittable
{
public:
	__device__ Sphere(const glm::vec3& center, float radius, Material* mat) :
		center(center), radius(radius < 0 ? 0 : radius), mat(mat) {}
	__device__ bool hit(const Ray& r, Interval rayT, hitData& data) const {
		//oc = C - Q
		glm::vec3 oc = center - r.origin();
		//elementy rownania kwadratowego
		float a = glm::dot(r.direction(), r.direction()); //To to samo co dlugosc^2
		float h = glm::dot(r.direction(), oc); //d = dir
		float c = glm::dot(oc, oc) - radius * radius;

		float delta = h * h - a * c;
		if (delta < 0)
			return false;

		float deltaSqr = sqrt(delta);
		//Nearest solution that lies within range
		float root = (h - deltaSqr) / a;
		if (!rayT.surrounds(root)) {
			root = (h + deltaSqr) / a;
			if (!rayT.surrounds(root))
				return false;
		}

		data.t = root;
		data.p = r.at(root);
		data.mat = mat;
		glm::dvec3 outwardNormal = (data.p - center) / radius;
		data.setFaceNormal(r, outwardNormal);

		return true;
	}

private:
	glm::vec3 center;
	float radius;
	Material* mat;
};