#pragma once
#include <curand_kernel.h>
#include "Utils.h"

class Interval {
public:
	__device__ Interval() : _min(+Utils::infinity), _max(-Utils::infinity) { }
	__device__ Interval(double min, double max) : _min(min), _max(max) {}

	__device__ double size() const {
		return _max - _min;
	}

	__device__ bool contains(double x) const {
		return _min <= x && _max >= x;
	}

	__device__ bool surrounds(double x) const {
		return _min < x && x < _max;
	}

	__device__ double clamp(double x) const {
		if (x < _min)
			return _min;
		if (x > _max)
			return _max;
		return x;
	}

	static const Interval empty, universe;
	double _min, _max;
};