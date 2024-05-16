#pragma once
#include "Ray.h"
#include <vector>

class HittableList : public Hittable {
public:
	std::vector<std::shared_ptr<Hittable>> objects;

	HittableList() {}
	HittableList(std::shared_ptr<Hittable> obj) { add(obj); }

	void clear() { objects.clear(); }

	void add(std::shared_ptr<Hittable> obj) {
		objects.push_back(obj);
	}
	
	bool hit(const Ray& r, Interval rayT, hitData& data) const {
		hitData tmp_data;
		bool hitAnything = false;
		double closestHit = rayT._max;

		for (const auto& obj : objects) {
			if (obj->hit(r, Interval(rayT._min, closestHit), tmp_data)) {
				hitAnything = true;
				closestHit = tmp_data.t;
				data = tmp_data;
			}
		}

		return hitAnything;
	}
};