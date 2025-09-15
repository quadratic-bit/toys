#pragma once
#include <cmath>

class Vector2 {
public:
	double x;
	double y;

	Vector2() : x(0), y(0) {}  // degenerate

	Vector2(double xx, double yy)
		: x(xx), y(yy) {}

	Vector2 perp_right() const {
		return Vector2(y, -x);
	}

	Vector2 perp_left() const {
		return Vector2(-y, x);
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

	Vector2 operator*(double scalar) const {
		return Vector2(x * scalar, y * scalar);
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
