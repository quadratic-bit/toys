#pragma once
#include "vectors.hpp"

class Sphere {
public:
	Vector3 pos;
	double radius;

	Sphere(const Vector3 &ppos, double rradius)
		: pos(ppos), radius(rradius) {}

	bool contains_2d(double x, double y) const {
		return (x - pos.x) * (x - pos.x) +
			(y - pos.y) * (y - pos.y) <= radius * radius;
	}

	double z_from_xy(double x, double y) const {
		assert(contains_2d(x, y));
		return std::sqrt(radius * radius -
				(x - pos.x) * (x - pos.x) -
				(y - pos.y) * (y - pos.y));
	}

	Vector3 normal(const Vector3 &point) const {
		return point;
	}
};
