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

	bool contains_2d(double x, double y) const {
		return (x - pos.x) * (x - pos.x) +
			(y - pos.y) * (y - pos.y) <= radius * radius;
	}

	bool contains(const Vector3 &point) const {
		Vector3 vec = point - pos;
		return (vec ^ vec) <= radius * radius;
	}

	bool intersects(const Vector3 &A, const Vector3& B) {
		Vector3 vec = B - A;
		double len_sqr = vec ^ vec;

		if (len_sqr == 0) {  // degenerate
			return contains(A);
		}

		double t = ((pos - A) ^ (vec)) / len_sqr;
		if (t < 0) t = 0;
		else if (t > 1) t = 1;

		Vector3 closest = A + vec * t;
		Vector3 dist = closest - pos;
		return (dist ^ dist) <= radius * radius;
	}

	double z_from_xy(double x, double y) const {
		assert(contains_2d(x, y));
		return std::sqrt(radius * radius -
				(x - pos.x) * (x - pos.x) -
				(y - pos.y) * (y - pos.y));
	}
};
