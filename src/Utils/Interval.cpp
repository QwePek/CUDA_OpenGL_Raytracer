#include "pch.h"
#include "Interval.h"
#include "Utils.h"

Interval::Interval() : _min(+Utils::infinity), _max(-Utils::infinity) { }
Interval::Interval(double min, double max) : _min(min), _max(max) {}

const Interval Interval::empty = Interval(+Utils::infinity, -Utils::infinity);
const Interval Interval::universe = Interval(-Utils::infinity, +Utils::infinity);
