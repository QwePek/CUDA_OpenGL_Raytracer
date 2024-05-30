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
	__device__ const double infinity = std::numeric_limits<double>::infinity();
    __device__ const double pi = 3.1415926535897932385;

    __device__ static inline double degToRad(double deg) {
		return deg * pi / 180.0;
	}

    __device__ static double generateRandomNumber(curandState* localRandState) {
        return curand_uniform_double(localRandState);
    }

    __device__ static double generateRandomNumber(float a, float b, curandState* localRandState) {
        // Generowanie i zwracanie losowej liczby
        return a + generateRandomNumber(localRandState) * (b - a);
    }

    namespace Vector {
        __device__ static inline double lenSquared(glm::dvec2 vec) {
            return vec.x * vec.x + vec.y * vec.y;
        }
        
        __device__ static inline double lenSquared(glm::dvec3 vec) {
            return vec.x * vec.x + vec.y * vec.y + vec.z * vec.z;
        }

        __device__ static glm::dvec3 randomVector(curandState* localRandState) {
            return glm::dvec3(generateRandomNumber(0.0, 1.0, localRandState), generateRandomNumber(0.0, 1.0, localRandState), 
                generateRandomNumber(0.0, 1.0, localRandState));
        }

        __device__ static glm::dvec3 randomVector(float a, float b, curandState* localRandState) {
            return glm::dvec3(generateRandomNumber(a, b, localRandState), generateRandomNumber(a, b, localRandState),
                generateRandomNumber(a, b, localRandState));
        }

        __device__ static inline glm::dvec3 randomInUnitSphere(curandState* localRandState) {
            while (true) {
                glm::dvec3 randVec = randomVector(-1.0f, 1.0f, localRandState);
                if (lenSquared(randVec) < 1)
                    return randVec;
            }
        }

        __device__ static inline glm::dvec3 randomInUnitSphereVector(curandState* localRandState) {
            return glm::normalize(randomInUnitSphere(localRandState));
        }

        __device__ static inline glm::dvec3 randomVectorOnHemisphere(const glm::dvec3& normal, curandState* localRandState) {
            glm::dvec3 onUnitSphere = randomInUnitSphereVector(localRandState);
            if (glm::dot(onUnitSphere, normal) > 0.0)
                return onUnitSphere;
            else
                return -onUnitSphere;
        }

        __device__ static inline bool nearZero(const glm::dvec3& vector) {
            double s = 1e-8;
            return (std::fabs(vector.x) < s) && (std::fabs(vector.y) < s) && (std::fabs(vector.z) < s);
        }

        __device__ static inline glm::dvec3 reflect(const glm::dvec3& v, const glm::dvec3& n) {
            return v - 2 * glm::dot(v, n) * n;
        }

        __device__ static inline glm::dvec3 refract(const glm::dvec3& uv, const glm::dvec3& n, double etaiOverEtat) {
            double cosAlpha = std::fmin(glm::dot(-uv, n), 1.0);
            glm::dvec3 outPerpendicular = etaiOverEtat * (uv + cosAlpha * n);
            glm::dvec3 outParallel = -sqrt(std::fabs(1.0 - lenSquared(outPerpendicular))) * n;
            return outPerpendicular + outParallel;
        }

        //To samo co randomInUnitSphere ale 2d
        __device__ static inline glm::dvec2 randomInUnitDisk(curandState* localRandState) {
            while (true) {
                glm::dvec2 p(generateRandomNumber(-1.0, 1.0, localRandState), generateRandomNumber(-1.0, 1.0, localRandState));
                if (lenSquared(p) < 1)
                    return p;
            }
        }
    }
}