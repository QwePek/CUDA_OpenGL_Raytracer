#pragma once
#include "Material.h"
#include "../../Camera.h"

namespace Materials
{
	class Dielectric : public Material
	{
	public:
		__device__ Dielectric(float refractionIndex) : refractionIndex(refractionIndex) { }

		__device__ bool scatter(const Ray& rayIn, const hitData& data, glm::vec3& attenuation, Ray& rayScattered, curandState* localRandState) const {
			attenuation = glm::vec3(1.0, 1.0, 1.0);
			float ri = data.frontFace ? (1.0 / refractionIndex) : refractionIndex;

			glm::vec3 unitdirection = glm::normalize(rayIn.direction());
			float cosalpha = std::fmin(glm::dot(-unitdirection, data.normal), 1.0f);
			float sinalpha = sqrt(1.0 - cosalpha * cosalpha);

			bool cannotrefract = ri * sinalpha > 1.0;
			glm::vec3 direction;

			if (cannotrefract || reflectance(cosalpha, ri) > Utils::generateRandomNumber(localRandState))
				direction = Utils::Vector::reflect(unitdirection, data.normal);
			else
				direction = Utils::Vector::refract(unitdirection, data.normal, ri);
			
			rayScattered = Ray(data.p, direction);
			return true;
		}

	private:
		float refractionIndex;

		__device__ static float reflectance(float cos, float refractionIndex) {
			float r0 = (1 - refractionIndex) / (1 + refractionIndex);
			r0 *= r0;
			return r0 + (1 - r0) * pow((1 - cos), 5);
		}
	};
}