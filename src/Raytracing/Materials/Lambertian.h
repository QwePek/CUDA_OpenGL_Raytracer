#pragma once
#include "Material.h"

namespace Materials
{
	class Lambertian : public Material
	{
	public:
		Lambertian(const glm::dvec3& albedo) : albedo(albedo) { }

		bool scatter(const Ray& rayIn, const hitData data, glm::dvec3& attenuation, Ray& rayScattered) const {
			glm::dvec3 scatterDirection = data.normal + Utils::Vector::randomInUnitSphereVector();

			//Obsluga tego jak random vector bedzie odwrotnoscia normali, wtedy moga sie pojawic rozne bledy :(
			if (Utils::Vector::nearZero(scatterDirection))
				scatterDirection = data.normal;

			rayScattered = Ray(data.p, scatterDirection);
			attenuation = albedo;

			return true;
		}

	private:
		glm::dvec3 albedo;
	};
}