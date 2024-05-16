#include "pch.h"
#include "Sphere.h"

bool Sphere::hit(const Ray& r, Interval rayT, hitData& data) const {
	//oc = C - Q
	glm::dvec3 oc = center - r.origin();
	//elementy rownania kwadratowego
	double a = glm::dot(r.direction(), r.direction()); //To to samo co dlugosc^2
	double h = glm::dot(r.direction(), oc); //d = dir
	double c = glm::dot(oc, oc) - radius * radius;

	double delta = h * h - a * c;
	if (delta < 0)
		return false;

	double deltaSqr = sqrt(delta);
	//Nearest solution that lies within range
	double root = (h - deltaSqr) / a;
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