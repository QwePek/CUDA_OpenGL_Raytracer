#pragma once
#include "../../Camera.h"

class Material
{
public:
	virtual ~Material() = default;

	virtual bool scatter(const Ray& rayIn, const hitData data, glm::dvec3& attenuation, Ray& rayScattered) const {
		return false;
	}
};