#pragma once
#include <cassert>

#include <swuix/common.hpp>

inline FRect frect(float x, float y, float w, float h) {
	FRect r;
	r.x = x;
	r.y = y;
	r.w = w;
	r.h = h;
	return r;
}

class Point2f {
public:
	float x;
	float y;

	Point2f() : x(0), y(0) {}  // degenerate

	explicit Point2f(float xx, float yy)
		: x(xx), y(yy) {}

	Point2f operator-() const {
		return Point2f(-x, -y);
	}

	Point2f operator-(const Point2f &other) const {
		return Point2f(x - other.x, y - other.y);
	}

	Point2f operator+(const Point2f &other) const {
		return Point2f(x + other.x, y + other.y);
	}

	void operator+=(const Point2f &other) {
		x += other.x;
		y += other.y;
	}

	void operator-=(const Point2f &other) {
		x -= other.x;
		y -= other.y;
	}
};
