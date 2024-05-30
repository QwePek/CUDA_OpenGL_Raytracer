#pragma once
#include "Material.h"
#include "../../Camera.h"

namespace Materials
{
	class Dielectric : public Material
	{
	public:
		__device__ Dielectric(double refractionIndex) : refractionIndex(refractionIndex) { }

		__device__ bool scatter(const Ray& rayIn, const hitData& data, glm::dvec3& attenuation, Ray& rayScattered, curandState* localRandState) const {
			attenuation = glm::dvec3(1.0, 1.0, 1.0);
			double ri = data.frontFace ? (1.0 / refractionIndex) : refractionIndex;

			glm::dvec3 unitdirection = glm::normalize(rayIn.direction());
			double cosalpha = std::fmin(glm::dot(-unitdirection, data.normal), 1.0);
			double sinalpha = sqrt(1.0 - cosalpha * cosalpha);

			bool cannotrefract = ri * sinalpha > 1.0;
			glm::dvec3 direction;

			if (cannotrefract || reflectance(cosalpha, ri) > Utils::generateRandomNumber(localRandState))
				direction = Utils::Vector::reflect(unitdirection, data.normal);
			else
				direction = Utils::Vector::refract(unitdirection, data.normal, ri);
			
			rayScattered = Ray(data.p, direction);
			return true;
		}

	private:
		double refractionIndex;

		__device__ static double reflectance(double cos, double refractionIndex) {
			double r0 = (1 - refractionIndex) / (1 + refractionIndex);
			r0 *= r0;
			return r0 + (1 - r0) * pow((1 - cos), 5);
		}
	};
}