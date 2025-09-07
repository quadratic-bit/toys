#pragma once
#include <cassert>
#include <math.h>

class Vector2 {
public:
	double x;
	double y;

	Vector2(double x_coord, double y_coord);

	void rotate(double angle);
};

class Vector3 {
public:
	double x;
	double y;
	double z;

	Vector3(double x_coord, double y_coord, double z_coord);
};

template<class T, unsigned int H, unsigned int W> class Matrix {
	T data[H][W];
public:
	Matrix(T data[H][W]);

	static Matrix<double, 2, 2> rotation(double rad) {
		double new_data[2][2], csn = std::cos(rad), sn = std::sin(rad);
		new_data[0][0] = csn;
		new_data[0][1] = -sn;
		new_data[1][0] = sn;
		new_data[1][1] = csn;
		return Matrix<double, 2, 2>(new_data);
	}

	T * at(unsigned int row, unsigned int col) {
		assert(row < H);
		assert(col < W);
		return data[row][col];
	}

	Matrix<T, H, W> operator+(const Matrix<T, H, W> &other) {
		T new_data[H][W];
		Matrix<T, H, W> new_mat(new_data);
		for (int row = 0; row < H; ++row) {
			for (int col = 0; col < W; ++col) {
				new_data[row][col] = this->data[row][col] + other.data[row][col];
			}
		}
	}

	Matrix<T, H, W> operator-(const Matrix<T, H, W> &other) {
		T new_data[H][W];
		Matrix<T, H, W> new_mat(new_data);
		for (int row = 0; row < H; ++row) {
			for (int col = 0; col < W; ++col) {
				new_data[row][col] = this->data[row][col] - other.data[row][col];
			}
		}
	}

	Matrix<T, H, W> operator*(int scalar) {
		T new_data[H][W];
		Matrix<T, H, W> new_mat(new_data);
		for (int row = 0; row < H; ++row) {
			for (int col = 0; col < W; ++col) {
				new_data[row][col] = this->data[row][col] * scalar;
			}
		}
	}

	Matrix<T, H, W> operator*(double scalar) {
		T new_data[H][W];
		Matrix<T, H, W> new_mat(new_data);
		for (int row = 0; row < H; ++row) {
			for (int col = 0; col < W; ++col) {
				new_data[row][col] = this->data[row][col] * scalar;
			}
		}
	}
};

template<class T, unsigned int H, unsigned int W>
Matrix<T, H, W> operator*(int scalar, Matrix<T, H, W> matrix) {
	return matrix * scalar;
}

template<class T, unsigned int H, unsigned int W>
Matrix<T, H, W> operator*(double scalar, Matrix<T, H, W> matrix) {
	return matrix * scalar;
}

typedef Matrix<double, 2, 2> Mat2;

// TODO: abolish
struct ArrowHead {
	Vector2 left;
	Vector2 right;
};
