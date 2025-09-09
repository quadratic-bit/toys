#pragma once

#include <cassert>
#include <cmath>

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
