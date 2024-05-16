#pragma once

class Interval {
public:
	Interval();
	Interval(double min, double max);

	double size() const {
		return _max - _min;
	}

	bool contains(double x) const {
		return _min <= x && _max >= x;
	}

	bool surrounds(double x) const {
		return _min < x && x < _max;
	}

	double clamp(double x) const {
		if (x < _min)
			return _min;
		if (x > _max)
			return _max;
		return x;
	}

	static const Interval empty, universe;
	double _min, _max;
};