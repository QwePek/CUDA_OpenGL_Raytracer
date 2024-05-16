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
    
    static inline double lenSquared(glm::vec3 vec) {
        return vec.x * vec.x + vec.y * vec.y + vec.z * vec.z;
    }

    static float generateRandomNumber(float a, float b) {
        // Tworzenie generatora liczb pseudolosowych
        std::uniform_real_distribution<float> distrib(a, b); // Dystrybucja równomierna typu float od a do b

        // Generowanie i zwracanie losowej liczby
        return distrib(gen);
    }

    static double generateRandomNumber(double a, double b) {
        std::uniform_real_distribution<double> distrib(a, b);

        return distrib(gen);
    }

    static glm::vec3 randomVector() {
        return glm::vec3(generateRandomNumber(0.0, 1.0), generateRandomNumber(0.0, 1.0), generateRandomNumber(0.0, 1.0));
    }

    static glm::vec3 randomVector(float a, float b) {
        return glm::vec3(generateRandomNumber(a, b), generateRandomNumber(a, b), generateRandomNumber(a, b));
    }

    static inline glm::vec3 randomInUnitSphere() {
        while (true) {
            glm::vec3 randVec = randomVector(-1.0f, 1.0f);
            if (lenSquared(randVec) < 1)
                return randVec;
        }
    }

    static inline glm::vec3 randomInUnitSphereVector() {
        return glm::normalize(randomInUnitSphere());
    }

    static inline glm::vec3 randomVectorOnHemisphere(const glm::vec3& normal) {
        glm::vec3 onUnitSphere = randomInUnitSphereVector();
        if (glm::dot(onUnitSphere, normal) > 0.0)
            return onUnitSphere;
        else
            return -onUnitSphere;
    }
}