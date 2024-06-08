#pragma once

class Material
{
public:
	__device__ virtual ~Material() = default;

	__device__ virtual bool scatter(const Ray& rayIn, const hitData& data, glm::vec3& attenuation, Ray& rayScattered, curandState* localRandState) const {
		return false;
	}
};