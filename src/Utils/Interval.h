#pragma once
#include "../Utils/Utils.h"

class Interval {
public:
	Interval();
	Interval(double min, double max) : min(min), max(max) {}

	double size() const {
		return max - min;
	}

	bool contains(double x) const {
		return min <= x && max >= x;
	}

	bool surrounds(double x) const {
		return min < x && x < max;
	}

	static const Interval empty, universe;

	double min, max;
};