#pragma once

#include <cmath>

#include "matrices.hpp"

class Vector2 {
public:
	double x;
	double y;

	Vector2(double xx, double yy)
		: x(xx), y(yy) {}

	Vector2(Matrix<2, 1> m) {
		this->x = m.at(0, 0);
		this->y = m.at(0, 1);
	}

	operator Matrix<2, 1>() {
		double data[2][1] = { { this->x }, { this->y } };
		return Matrix<2, 1>(data);
	}

	void rotate(double angle) {
		Mat2 rot = Mat2::rotation(angle);
		Matrix<2, 1> res = rot * (Matrix<2, 1>)(*this);
		x = res.at(0, 0);
		y = res.at(1, 0);
	}

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

	Vector2 operator+(const Vector2 &other) const {
		return Vector2(x + other.x, y + other.y);
	}

	Vector2 operator*(double scalar) const {
		return Vector2(x * scalar, y * scalar);
	}

	Vector2 operator/(double scalar) const {
		return Vector2(x / scalar, y / scalar);
	}

	Vector2 operator!() const {
		return *this / this->length();
	}
};

class Vector3 {
public:
	double x;
	double y;
	double z;

	Vector3(double xx, double yy, double zz)
		: x(xx), y(yy), z(zz) {}

	Vector3(Matrix<3, 1> m) {
		this->x = m.at(0, 0);
		this->y = m.at(0, 1);
		this->z = m.at(0, 2);
	}

	double length() const {
		return std::sqrt(x * x + y * y + z * z);
	}

	void rotate_xz(double angle) {
		Vector2 xz(x, z);
		xz.rotate(angle);
		x = xz.x;
		z = xz.y;
	}

	//  ^       ^        ^
	//   \      |       /
	//    \     |      /
	// this  normal  result
	Vector3 reflect(const Vector3 * const normal) {
		return *normal * 2 - *this;
	}

	Vector3 operator-() const {
		return Vector3(-x, -y, -z);
	}

	Vector3 operator-(const Vector3 &other) const {
		return Vector3(x - other.x, y - other.y, z - other.z);
	}

	Vector3 operator+() const {
		return *this;
	}

	Vector3 operator+(const Vector3 &other) const {
		return Vector3(x + other.x, y + other.y, z + other.z);
	}

	Vector3 operator/(double scalar) const {
		return Vector3(x / scalar, y / scalar, z / scalar);
	}

	Vector3 operator*(double scalar) const {
		return Vector3(x * scalar, y * scalar, z * scalar);
	}

	double operator^(const Vector3 &right) const {
		return x * right.x + y * right.y + z * right.z;
	}

	Vector3 operator!() const {
		return *this / this->length();
	}

	operator Matrix<3, 1>() {
		double data[3][1] = { { this->x }, { this->y }, { this->z } };
		return Matrix<3, 1>(data);
	}
};

Vector3 operator*(double left, const Vector3 &right);
Vector2 operator*(double left, const Vector2 &right);
