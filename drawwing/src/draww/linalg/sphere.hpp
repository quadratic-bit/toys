#pragma once
#include "vectors.hpp"

class Sphere {
public:
	Vector3 pos;
	double radius;

	Sphere(const Vector3 &ppos, double rradius)
		: pos(ppos), radius(rradius) {}

	Vector3 normal(const Vector3 &point) const {
		return point - pos;
	}

	double z_from_xy(double x, double y) const {
		const double dx = x - pos.x, dy = y - pos.y;
		double d2 = radius * radius - dx * dx - dy * dy;

		if (d2 < 0.0 && d2 > -1e-12) d2 = 0.0;

		assert(d2 >= 0.0);

		return pos.z + std::sqrt(d2);
	}
};
