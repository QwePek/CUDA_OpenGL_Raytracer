#include "pch.h"
#include "Interval.h"
#include "Utils.h"

const Interval Interval::empty = Interval(+Utils::infinity, -Utils::infinity);
const Interval Interval::universe = Interval(-Utils::infinity, +Utils::infinity);
