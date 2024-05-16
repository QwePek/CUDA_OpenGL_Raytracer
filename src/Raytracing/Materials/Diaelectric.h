#pragma once
#include "Material.h"

namespace Materials
{
	class Diaelectric : public Material
	{
	public:
		Diaelectric(double refractionIndex) : refractionIndex(refractionIndex) { }

		bool scatter(const Ray& rayIn, const hitData data, glm::dvec3& attenuation, Ray& rayScattered) const {
			attenuation = glm::dvec3(1.0, 1.0, 1.0);
			double ri = data.frontFace ? (1.0 / refractionIndex) : refractionIndex;

			//glm::dvec3 unitDirection = glm::dot(rayIn.direction());
			return true;
		}

	private:
		double refractionIndex;
	}
}