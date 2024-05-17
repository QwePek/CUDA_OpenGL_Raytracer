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

namespace Utils {
	const double infinity = std::numeric_limits<double>::infinity();
	const double pi = 3.1415926535897932385;
    static std::random_device rd;
    static std::mt19937 gen(rd());

	static inline double degToRad(double deg) {
		return deg * pi / 180.0;
	}

    static float generateRandomNumber(float a, float b) {
        // Tworzenie generatora liczb pseudolosowych
        std::uniform_real_distribution<float> distrib(a, b); // Dystrybucja równomierna typu float od a do b

        // Generowanie i zwracanie losowej liczby
        return distrib(gen);
    }

    static double generateRandomNumber() {
        std::uniform_real_distribution<double> distrib(0.0, 1.0);

        return distrib(gen);
    }
    
    static double generateRandomNumber(double a, double b) {
        std::uniform_real_distribution<double> distrib(a, b);

        return distrib(gen);
    }

    namespace Vector {
        static inline double lenSquared(glm::dvec2 vec) {
            return vec.x * vec.x + vec.y * vec.y;
        }
        
        static inline double lenSquared(glm::dvec3 vec) {
            return vec.x * vec.x + vec.y * vec.y + vec.z * vec.z;
        }

        static glm::dvec3 randomVector() {
            return glm::dvec3(generateRandomNumber(0.0, 1.0), generateRandomNumber(0.0, 1.0), generateRandomNumber(0.0, 1.0));
        }

        static glm::dvec3 randomVector(float a, float b) {
            return glm::dvec3(generateRandomNumber(a, b), generateRandomNumber(a, b), generateRandomNumber(a, b));
        }

        static inline glm::dvec3 randomInUnitSphere() {
            while (true) {
                glm::dvec3 randVec = randomVector(-1.0f, 1.0f);
                if (lenSquared(randVec) < 1)
                    return randVec;
            }
        }

        static inline glm::dvec3 randomInUnitSphereVector() {
            return glm::normalize(randomInUnitSphere());
        }

        static inline glm::dvec3 randomVectorOnHemisphere(const glm::dvec3& normal) {
            glm::dvec3 onUnitSphere = randomInUnitSphereVector();
            if (glm::dot(onUnitSphere, normal) > 0.0)
                return onUnitSphere;
            else
                return -onUnitSphere;
        }

        static inline bool nearZero(const glm::dvec3& vector) {
            double s = 1e-8;
            return (std::fabs(vector.x) < s) && (std::fabs(vector.y) < s) && (std::fabs(vector.z) < s);
        }

        static inline glm::dvec3 reflect(const glm::dvec3& v, const glm::dvec3& n) {
            return v - 2 * glm::dot(v, n) * n;
        }

        static inline glm::dvec3 refract(const glm::dvec3& uv, const glm::dvec3& n, double etaiOverEtat) {
            double cosAlpha = std::fmin(glm::dot(-uv, n), 1.0);
            glm::dvec3 outPerpendicular = etaiOverEtat * (uv + cosAlpha * n);
            glm::dvec3 outParallel = -sqrt(std::fabs(1.0 - lenSquared(outPerpendicular))) * n;
            return outPerpendicular + outParallel;
        }

        //To samo co randomInUnitSphere ale 2d
        static inline glm::dvec2 randomInUnitDisk() {
            while (true) {
                glm::dvec2 p(generateRandomNumber(-1.0, 1.0), generateRandomNumber(-1.0, 1.0));
                if (lenSquared(p) < 1)
                    return p;
            }
        }
    }
}