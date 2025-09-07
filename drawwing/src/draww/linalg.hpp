#pragma once
#include <cassert>
#include <cmath>
#include <math.h>

template<unsigned int H, unsigned int W> class Matrix {
	double data[H][W];
public:
	Matrix(const double (&m)[H][W]) {
		for (unsigned int row = 0; row < H; ++row) {
			for (unsigned int col = 0; col < W; ++col) {
				data[row][col] = m[row][col];
			}
		}
	}

	static Matrix<2, 2> rotation(double rad) {
		double csn = std::cos(rad), sn = std::sin(rad);
		double new_data[2][2] = { { csn, -sn }, { sn, csn } };
		return Matrix<2, 2>(new_data);
	}

	double *mut_at(unsigned int row, unsigned int col) {
		assert(row < H);
		assert(col < W);
		return &data[row][col];
	}

	double at(unsigned int row, unsigned int col) const {
		assert(row < H);
		assert(col < W);
		return data[row][col];
	}

	Matrix<H, W> operator+(const Matrix<H, W> &other) {
		double new_data[H][W];
		Matrix<H, W> new_mat(new_data);
		for (unsigned int row = 0; row < H; ++row) {
			for (unsigned int col = 0; col < W; ++col) {
				new_data[row][col] = this->data[row][col] + other.at(row, col);
			}
		}
		return new_mat;
	}

	Matrix<H, W> operator-(const Matrix<H, W> &other) {
		double new_data[H][W];
		Matrix<H, W> new_mat(new_data);
		for (unsigned int row = 0; row < H; ++row) {
			for (unsigned int col = 0; col < W; ++col) {
				new_data[row][col] = this->data[row][col] - other.at(row, col);
			}
		}
		return new_mat;
	}

	Matrix<H, W> operator*(double scalar) {
		double new_data[H][W];
		Matrix<H, W> new_mat(new_data);
		for (unsigned int row = 0; row < H; ++row) {
			for (unsigned int col = 0; col < W; ++col) {
				new_data[row][col] = this->data[row][col] * scalar;
			}
		}
		return new_mat;
	}

	template<unsigned int D>
	Matrix<H, D> operator*(const Matrix<W, D> &other) {
		double new_data[H][D] = {{0}};
		Matrix<H, D> new_mat(new_data);
		for (unsigned int row = 0; row < H; ++row) {
			for (unsigned int col = 0; col < D; ++col) {
				for (unsigned int i = 0; i < W; ++i) {
					*new_mat.mut_at(row, col) += this->data[row][i] * other.at(i, col);
				}
			}
		}
		return new_mat;
	}
};

template<unsigned int H, unsigned int W>
Matrix<H, W> operator*(double scalar, Matrix<H, W> matrix) {
	return matrix * scalar;
}

typedef Matrix<2, 2> Mat2;

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

class Sphere {
public:
	Vector3 pos;
	double radius;

	Sphere(const Vector3 &ppos, double rradius)
		: pos(ppos), radius(rradius) {}

	bool contains_2d(double x, double y) const {
		return x * x + y * y <= radius * radius;
	}

	double z_from_xy(double x, double y) const {
		assert(contains_2d(x, y));
		return std::sqrt(radius * radius - x * x - y * y);
	}

	Vector3 normal(const Vector3 &point) const {
		return !point;
	}
};

// TODO: abolish
struct ArrowHead {
	Vector2 left;
	Vector2 right;
};
