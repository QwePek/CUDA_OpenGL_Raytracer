#pragma once
#include "glm\glm.hpp"
#include "glm\gtc\matrix_transform.hpp"
#include "glm\gtc\type_ptr.hpp"

#include <iostream>
#include <memory>
#include <cmath>
#include <limits>

#include "Interval.h"

namespace Utils {
	const double infinity = std::numeric_limits<double>::infinity();
	const double pi = 3.1415926535897932385;

	inline double degToRad(double deg) {
		return deg * pi / 180.0;
	}
}