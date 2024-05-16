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

    static double generateRandomNumber(double a, double b) {
        std::uniform_real_distribution<double> distrib(a, b);

        return distrib(gen);
    }
}