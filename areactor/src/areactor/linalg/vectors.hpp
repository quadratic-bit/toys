#pragma once
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <SDL3/SDL_rect.h>

inline float rand01() {
	return (float)(std::rand()) / RAND_MAX;
}

inline float sign(float a) {
	return (float)(0 < a) - (float)(a < 0);
}

class Vec2f {
public:
	float x;
	float y;

	Vec2f() : x(0), y(0) {}  // degenerate

	explicit Vec2f(float xx, float yy)
		: x(xx), y(yy) {}

	static Vec2f random_rect(float w, float h) {
		assert(!std::isnan(w));
		assert(!std::isinf(w));
		assert(!std::isnan(h));
		assert(!std::isinf(h));
		assert(w >= 0.0);
		assert(h >= 0.0);
		return Vec2f(rand01() * w, rand01() * h);
	}

	static Vec2f random_radial(float min_mag, float max_mag) {
		assert(!std::isnan(min_mag));
		assert(!std::isinf(min_mag));
		assert(!std::isnan(max_mag));
		assert(!std::isinf(max_mag));
		assert(min_mag >= 0.0);
		assert(max_mag >= min_mag);

		float mag = (max_mag - min_mag) * rand01() + min_mag;
		float angle = 2 * M_PI * rand01();

		float x = mag * std::cos(angle);
		float y = mag * std::sin(angle);

		return Vec2f(x, y);
	}

	Vec2f rotate(float ang) const {
		const float c = std::cos(ang), s = std::sin(ang);
		return Vec2f(c * x - s * y, s * x + c * y);
	}

	Vec2f perp_right() const {
		return Vec2f(y, -x);
	}

	Vec2f perp_left() const {
		return Vec2f(-y, x);
	}

	Vec2f norm_or(const Vec2f &fallback) const {
		float L = length();
		if (L == 0.0) return fallback;
		return *this / L;
	}

	float length() const {
		return std::sqrt(x * x + y * y);
	}

	Vec2f operator-() const {
		return Vec2f(-x, -y);
	}

	Vec2f operator-(const Vec2f &other) const {
		return Vec2f(x - other.x, y - other.y);
	}

	Vec2f operator+(const Vec2f &other) const {
		return Vec2f(x + other.x, y + other.y);
	}

	void operator+=(const Vec2f &other) {
		x += other.x;
		y += other.y;
	}

	Vec2f operator*(float scalar) const {
		return Vec2f(x * scalar, y * scalar);
	}

	float operator^(const Vec2f &other) const {
		return x * other.x + y * other.y;
	}

	friend Vec2f operator*(float left, const Vec2f &right) {
		return right * left;
	}

	Vec2f operator/(float scalar) const {
		return Vec2f(x / scalar, y / scalar);
	}

	void operator/=(float scalar) {
		this->x /= scalar;
		this->y /= scalar;
	}

	Vec2f operator!() const {
		return *this / this->length();
	}
};
