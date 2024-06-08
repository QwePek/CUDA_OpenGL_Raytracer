#pragma once
#include "Material.h"
#include "../../Camera.h"

namespace Materials
{
	class Lambertian : public Material
	{
	public:
		__device__ Lambertian(const glm::vec3& albedo) : albedo(albedo) { }

		__device__ bool scatter(const Ray& rayIn, const hitData& data, glm::vec3& attenuation, Ray& rayScattered, curandState* localRandState) const {
			glm::vec3 scatterDirection = data.normal + Utils::Vector::randomInUnitSphereVector(localRandState);

			//Obsluga tego jak random vector bedzie odwrotnoscia normali, wtedy moga sie pojawic rozne bledy :(
			if (Utils::Vector::nearZero(scatterDirection))
				scatterDirection = data.normal;

			rayScattered = Ray(data.p, scatterDirection);
			attenuation = albedo;

			return true;
		}

	private:
		glm::vec3 albedo;
	};
}