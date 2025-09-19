#pragma once
#include <cassert>
#include <cmath>
#include <cstdlib>

inline double rand01() {
	return (double)(std::rand()) / RAND_MAX;
}

inline double sign(double a) {
	return (double)(0 < a) - (double)(a < 0);
}

class Vector2 {
public:
	double x;
	double y;

	Vector2() : x(0), y(0) {}  // degenerate

	explicit Vector2(double xx, double yy)
		: x(xx), y(yy) {}

	static Vector2 random_rect(double w, double h) {
		assert(!std::isnan(w));
		assert(!std::isinf(w));
		assert(!std::isnan(h));
		assert(!std::isinf(h));
		assert(w >= 0.0);
		assert(h >= 0.0);
		return Vector2(rand01() * w, rand01() * h);
	}

	static Vector2 random_radial(double min_mag, double max_mag) {
		assert(!std::isnan(min_mag));
		assert(!std::isinf(min_mag));
		assert(!std::isnan(max_mag));
		assert(!std::isinf(max_mag));
		assert(min_mag >= 0.0);
		assert(max_mag >= min_mag);

		double mag = (max_mag - min_mag) * rand01() + min_mag;
		double angle = 2 * M_PI * rand01();

		double x = mag * std::cos(angle);
		double y = mag * std::sin(angle);

		return Vector2(x, y);
	}

	Vector2 rotate(double ang) const {
		const double c = std::cos(ang), s = std::sin(ang);
		return Vector2(c * x - s * y, s * x + c * y);
	}

	Vector2 perp_right() const {
		return Vector2(y, -x);
	}

	Vector2 perp_left() const {
		return Vector2(-y, x);
	}

	Vector2 norm_or(const Vector2 &fallback) const {
		double L = length();
		if (L == 0.0) return fallback;
		return *this / L;
	}

	double length() const {
		return std::sqrt(x * x + y * y);
	}

	Vector2 operator-() const {
		return Vector2(-x, -y);
	}

	Vector2 operator-(const Vector2 &other) const {
		return Vector2(x - other.x, y - other.y);
	}

	Vector2 operator+(const Vector2 &other) const {
		return Vector2(x + other.x, y + other.y);
	}

	void operator+=(const Vector2 &other) {
		x += other.x;
		y += other.y;
	}

	Vector2 operator*(double scalar) const {
		return Vector2(x * scalar, y * scalar);
	}

	double operator^(const Vector2 &other) const {
		return x * other.x + y * other.y;
	}

	friend Vector2 operator*(double left, const Vector2 &right) {
		return right * left;
	}

	Vector2 operator/(double scalar) const {
		return Vector2(x / scalar, y / scalar);
	}

	Vector2 operator!() const {
		return *this / this->length();
	}
};
