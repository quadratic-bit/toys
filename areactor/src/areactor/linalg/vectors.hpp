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

class Vec2 {
public:
	float x;
	float y;

	Vec2() : x(0), y(0) {}  // degenerate

	explicit Vec2(float xx, float yy)
		: x(xx), y(yy) {}

	static Vec2 random_rect(float w, float h) {
		assert(!std::isnan(w));
		assert(!std::isinf(w));
		assert(!std::isnan(h));
		assert(!std::isinf(h));
		assert(w >= 0.0);
		assert(h >= 0.0);
		return Vec2(rand01() * w, rand01() * h);
	}

	static Vec2 random_radial(float min_mag, float max_mag) {
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

		return Vec2(x, y);
	}

	Vec2 rotate(float ang) const {
		const float c = std::cos(ang), s = std::sin(ang);
		return Vec2(c * x - s * y, s * x + c * y);
	}

	Vec2 perp_right() const {
		return Vec2(y, -x);
	}

	Vec2 perp_left() const {
		return Vec2(-y, x);
	}

	Vec2 norm_or(const Vec2 &fallback) const {
		float L = length();
		if (L == 0.0) return fallback;
		return *this / L;
	}

	float length() const {
		return std::sqrt(x * x + y * y);
	}

	Vec2 operator-() const {
		return Vec2(-x, -y);
	}

	Vec2 operator-(const Vec2 &other) const {
		return Vec2(x - other.x, y - other.y);
	}

	Vec2 operator+(const Vec2 &other) const {
		return Vec2(x + other.x, y + other.y);
	}

	void operator+=(const Vec2 &other) {
		x += other.x;
		y += other.y;
	}

	Vec2 operator*(float scalar) const {
		return Vec2(x * scalar, y * scalar);
	}

	float operator^(const Vec2 &other) const {
		return x * other.x + y * other.y;
	}

	friend Vec2 operator*(float left, const Vec2 &right) {
		return right * left;
	}

	Vec2 operator/(float scalar) const {
		return Vec2(x / scalar, y / scalar);
	}

	void operator/=(float scalar) {
		this->x /= scalar;
		this->y /= scalar;
	}

	Vec2 operator!() const {
		return *this / this->length();
	}
};

inline SDL_FRect frect(float x, float y, float w, float h) {
	SDL_FRect r;
	r.x = x;
	r.y = y;
	r.w = w;
	r.h = h;
	return r;
}
