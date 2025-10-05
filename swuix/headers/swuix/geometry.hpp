#pragma once
#include <algorithm>
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

inline FRect intersect(const FRect &a, const FRect &b) {
	float left  = std::max(a.x, b.x);
	float top   = std::max(a.y, b.y);
	float right = std::min(a.x + a.w, b.x + b.w);
	float btm   = std::min(a.y + a.h, b.y + b.h);

	float cross_width = right - left;
	float cross_height = btm - top;
	if (cross_width <= 0.0 || cross_height <= 0.0) return frect(0, 0, 0, 0);
	return frect(left, top, cross_width, cross_height);
}

inline float clamp(float value, float lower, float upper) {
	return std::min(std::max(value, lower), upper);
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

	bool operator==(const Point2f &other) {
		return x == other.x && y == other.y;
	}
};
