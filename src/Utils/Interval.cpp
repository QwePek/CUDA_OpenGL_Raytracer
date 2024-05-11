#include "pch.h"
#include "Interval.h"

Interval::Interval() : min(+Utils::infinity), max(-Utils::infinity) {
}


const Interval Interval::empty = Interval(+Utils::infinity, -Utils::infinity);
const Interval Interval::universe = Interval(-Utils::infinity, +Utils::infinity);
