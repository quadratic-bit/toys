#pragma once
#include <SDL3/SDL_rect.h>
#include <cassert>
#include <cmath>

inline SDL_FRect frect(float x, float y, float w, float h) {
	SDL_FRect r;
	r.x = x;
	r.y = y;
	r.w = w;
	r.h = h;
	return r;
}

class Vec2 {
public:
	float x;
	float y;

	Vec2() : x(0), y(0) {}  // degenerate

	explicit Vec2(float xx, float yy)
		: x(xx), y(yy) {}


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
