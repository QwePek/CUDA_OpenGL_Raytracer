#pragma once
#include <curand_kernel.h>
#include "Utils.h"

class Interval {
public:
	__device__ Interval() : _min(+Utils::infinity), _max(-Utils::infinity) { }
	__device__ Interval(float min, float max) : _min(min), _max(max) {}

	__device__ float size() const {
		return _max - _min;
	}

	__device__ bool contains(float x) const {
		return _min <= x && _max >= x;
	}

	__device__ bool surrounds(float x) const {
		return _min < x && x < _max;
	}

	__device__ float clamp(float x) const {
		if (x < _min)
			return _min;
		if (x > _max)
			return _max;
		return x;
	}

	static const Interval empty, universe;
	float _min, _max;
};