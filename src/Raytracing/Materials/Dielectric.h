#pragma once
#include "Material.h"

namespace Materials
{
	class Dielectric : public Material
	{
	public:
		Dielectric(double refractionIndex) : refractionIndex(refractionIndex) { }

		bool scatter(const Ray& rayIn, const hitData& data, glm::dvec3& attenuation, Ray& rayScattered) const {

			//attenuation = glm::dvec3(1.0, 1.0, 1.0);
			//double ri = rec.frontFace ? (1.0 / refractionIndex) : refractionIndex;

			//glm::dvec3 unit_direction = glm::normalize(r_in.direction());
			//double cos_theta = std::fmin(glm::dot(-unit_direction, rec.normal), 1.0);
			//double sin_theta = sqrt(1.0 - cos_theta * cos_theta);

			//bool cannot_refract = ri * sin_theta > 1.0;
			//glm::dvec3 direction;

			//if (cannot_refract)
			//	direction = Utils::Vector::reflect(unit_direction, rec.normal);
			//else
			//	direction = Utils::Vector::refract(unit_direction, rec.normal, ri);

			//scattered = Ray(rec.p, direction);
			//return true;
			attenuation = glm::dvec3(1.0, 1.0, 1.0);
			double ri = data.frontFace ? (1.0 / refractionIndex) : refractionIndex;

			glm::dvec3 unitdirection = glm::normalize(rayIn.direction());
			double cosalpha = std::fmin(glm::dot(-unitdirection, data.normal), 1.0);
			double sinalpha = sqrt(1.0 - cosalpha * cosalpha);

			bool cannotrefract = ri * sinalpha > 1.0;
			glm::dvec3 direction;

			if (cannotrefract || reflectance(cosalpha, ri) > Utils::generateRandomNumber())
				direction = Utils::Vector::reflect(unitdirection, data.normal);
			else
				direction = Utils::Vector::refract(unitdirection, data.normal, ri);
			
			rayScattered = Ray(data.p, direction);
			return true;
		}

	private:
		double refractionIndex;

		static double reflectance(double cos, double refractionIndex) {
			double r0 = (1 - refractionIndex) / (1 + refractionIndex);
			r0 *= r0;
			return r0 + (1 - r0) * pow((1 - cos), 5);
		}
	};
}