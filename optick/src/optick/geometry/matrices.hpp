#pragma once
#include <cassert>
#include <cmath>

template<unsigned int H, unsigned int W>
class Matrix {
	template<unsigned int, unsigned int> friend class Matrix;
	double data[H][W];

public:
	// TODO: explicit
	Matrix(const double m[H][W]) {
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

	static Matrix zero() {
		double data[H][W] = {0};
		return Matrix(data);
	}

	static Matrix<H, H> id() {
		double data[H][H] = {0};
		for (int i = 0; i < H; ++i) {
			data[i][i] = 1.0;
		}
		return Matrix<H, H>(data);
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

	Matrix<H, W> operator+(const Matrix<H, W> &other) const {
		double new_data[H][W];
		for (unsigned int row = 0; row < H; ++row) {
			for (unsigned int col = 0; col < W; ++col) {
				new_data[row][col] = this->data[row][col] + other.data[row][col];
			}
		}
		return Matrix<H, W>(new_data);
;
	}

	Matrix<H, W> operator-(const Matrix<H, W> &other) const {
		double new_data[H][W];
		for (unsigned int row = 0; row < H; ++row) {
			for (unsigned int col = 0; col < W; ++col) {
				new_data[row][col] = this->data[row][col] - other.data[row][col];
			}
		}
		return Matrix<H, W>(new_data);
	}

	Matrix<H, W> operator*(double scalar) const {
		double new_data[H][W];
		for (unsigned int row = 0; row < H; ++row) {
			for (unsigned int col = 0; col < W; ++col) {
				new_data[row][col] = this->data[row][col] * scalar;
			}
		}
		return Matrix<H, W>(new_data);
	}

	template<unsigned int D>
	Matrix<H, D> operator*(const Matrix<W, D> &other) const {
		double new_data[H][D] = {{0}};
		for (unsigned int row = 0; row < H; ++row) {
			for (unsigned int col = 0; col < D; ++col) {
				for (unsigned int i = 0; i < W; ++i) {
					new_data[row][col] += this->data[row][i] * other.data[i][col];
				}
			}
		}
		return Matrix<H, W>(new_data);
	}

	friend Matrix<H, W> operator*(double scalar, Matrix<H, W> matrix) {
		return matrix * scalar;
	}
};

typedef Matrix<2, 2> Mat2;
typedef Matrix<3, 3> Mat3;
