#pragma once
#include "glm\glm.hpp"
#include "glm\gtc\matrix_transform.hpp"
#include "glm\gtc\type_ptr.hpp"

#include <iostream>
#include <memory>
#include <cmath>
#include <limits>
#include <cstdlib>
#include <random>
#include <ctime>
#include <curand_kernel.h>

namespace Utils {
	__device__ const float infinity = std::numeric_limits<float>::infinity();
    __device__ const float pi = 3.1415926535897932385;

    __device__ static inline double degToRad(double deg) {
		return deg * pi / 180.0;
	}

    __device__ static float generateRandomNumber(curandState* localRandState) {
        return curand_uniform(localRandState);
    }

    __device__ static float generateRandomNumber(float a, float b, curandState* localRandState) {
        // Generowanie i zwracanie losowej liczby
        return a + generateRandomNumber(localRandState) * (b - a);
    }

    namespace Vector {
        __device__ static inline float lenSquared(glm::vec2 vec) {
            return vec.x * vec.x + vec.y * vec.y;
        }
        
        __device__ static inline float lenSquared(glm::vec3 vec) {
            return vec.x * vec.x + vec.y * vec.y + vec.z * vec.z;
        }

        __device__ static glm::vec3 randomVector(curandState* localRandState) {
            return glm::vec3(generateRandomNumber(0.0, 1.0, localRandState), generateRandomNumber(0.0, 1.0, localRandState), 
                generateRandomNumber(0.0, 1.0, localRandState));
        }

        __device__ static glm::vec3 randomVector(float a, float b, curandState* localRandState) {
            return glm::vec3(generateRandomNumber(a, b, localRandState), generateRandomNumber(a, b, localRandState),
                generateRandomNumber(a, b, localRandState));
        }

        __device__ static inline glm::vec3 randomInUnitSphere(curandState* localRandState) {
            while (true) {
                glm::vec3 randVec = randomVector(-1.0f, 1.0f, localRandState);
                if (lenSquared(randVec) < 1)
                    return randVec;
            }
        }

        __device__ static inline glm::vec3 randomInUnitSphereVector(curandState* localRandState) {
            return glm::normalize(randomInUnitSphere(localRandState));
        }

        __device__ static inline glm::vec3 randomVectorOnHemisphere(const glm::vec3& normal, curandState* localRandState) {
            glm::vec3 onUnitSphere = randomInUnitSphereVector(localRandState);
            if (glm::dot(onUnitSphere, normal) > 0.0f)
                return onUnitSphere;
            else
                return -onUnitSphere;
        }

        __device__ static inline bool nearZero(const glm::vec3& vector) {
            float s = 1e-8f;
            return (std::fabs(vector.x) < s) && (std::fabs(vector.y) < s) && (std::fabs(vector.z) < s);
        }

        __device__ static inline glm::vec3 reflect(const glm::vec3& v, const glm::vec3& n) {
            return v - 2 * glm::dot(v, n) * n;
        }

        __device__ static inline glm::vec3 refract(const glm::vec3& uv, const glm::vec3& n, float etaiOverEtat) {
            float cosAlpha = std::fmin(glm::dot(-uv, n), 1.0f);
            glm::vec3 outPerpendicular = etaiOverEtat * (uv + cosAlpha * n);
            glm::vec3 outParallel = -sqrt(std::fabs(1.0f - lenSquared(outPerpendicular))) * n;
            return outPerpendicular + outParallel;
        }

        //To samo co randomInUnitSphere ale 2d
        __device__ static inline glm::vec2 randomInUnitDisk(curandState* localRandState) {
            while (true) {
                glm::vec2 p(generateRandomNumber(-1.0f, 1.0f, localRandState), generateRandomNumber(-1.0f, 1.0f, localRandState));
                if (lenSquared(p) < 1)
                    return p;
            }
        }
    }
}