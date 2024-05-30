#pragma once
#include "Material.h"
#include "../../Camera.h"

namespace Materials
{
	class Metal : public Material
	{
	public:
		__device__ Metal(const glm::dvec3& albedo, double fuzz) : albedo(albedo), fuzz(fuzz < 1 ? fuzz : 1) { }

		__device__ bool scatter(const Ray& rayIn, const hitData& data, glm::dvec3& attenuation, Ray& rayScattered, curandState* localRandState) const {
			glm::dvec3 reflected = Utils::Vector::reflect(rayIn.direction(), data.normal);
			reflected = glm::normalize(reflected) + (fuzz * Utils::Vector::randomInUnitSphereVector(localRandState));

			rayScattered = Ray(data.p, reflected);
			attenuation = albedo;

			return (glm::dot(rayScattered.direction(), data.normal) > 0);
		}

	private:
		glm::dvec3 albedo;
		double fuzz;
	};
}