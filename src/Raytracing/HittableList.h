#pragma once
#include "Hittable.h"
#include <vector>

class HittableList : public Hittable {
public:
	Hittable** objects = nullptr;
	int objectsSize = 1;

	__device__ HittableList() {}
	__device__ HittableList(Hittable** o, int size) { objects = o; objectsSize = size; }
	__host__ ~HittableList() { delete []objects; }

	__device__ bool hit(const Ray& r, Interval rayT, hitData& data) const {
		hitData tmp_data;
		bool hitAnything = false;
		double closestHit = rayT._max;

		for (int i = 0; i < objectsSize; i++) {
			if (objects[i]->hit(r, Interval(rayT._min, closestHit), tmp_data)) {
				hitAnything = true;
				closestHit = tmp_data.t;
				data = tmp_data;
			}
		}

		return hitAnything;
	}
};