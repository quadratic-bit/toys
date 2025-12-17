#pragma once
#include <algorithm>

#include "./matrices.hpp"

inline double clampd(double value, double min, double max) {
    return std::max(min, std::min(max, value));
}

inline float rand01() {
    return (float)(std::rand()) / RAND_MAX;
}

inline double deg2rad(double d) {
    return d * (M_PI / 180.0);
}

inline double clamp(double v, double lo, double hi) {
    return v < lo ? lo : (v > hi ? hi : v);
}

class Vector3 {
public:
    double x;
    double y;
    double z;

    Vector3() : x(0), y(0), z(0) {}  // degenerate

    Vector3(double xx, double yy, double zz)
        : x(xx), y(yy), z(zz) {}

    Vector3(Matrix<3, 1> m) {
        this->x = m.at(0, 0);
        this->y = m.at(0, 1);
        this->z = m.at(0, 2);
    }

    operator Matrix<3, 1>() {
        double data[3][1] = { { this->x }, { this->y }, { this->z } };
        return Matrix<3, 1>(data);
    }

    double length() const {
        return std::sqrt(*this ^ *this);
    }

    static Vector3 reflect(const Vector3 &vec, const Vector3 &norm) {
        return vec - norm * (2.0 * (vec ^ norm));
    }

    bool refract(const Vector3 &vec, const Vector3 &norm, double etai, double etat) {
        double cos_incident = clampd(vec ^ norm, -1.0, +1.0);
        Vector3 n = norm;
        double eta = etai / etat;
        if (cos_incident > 0.0) {  // inside; invert normal
            n = norm * -1.0;
        }
        double k = 1.0 - eta * eta * (1.0 - cos_incident * cos_incident);
        if (k < 0.0) return false; // TIR
        *this = !(vec * eta - n * (eta * cos_incident + std::sqrt(k)));
        return true;
    }

    // zero if input is zero
    inline Vector3 normalizeClamp() {
        const double len2 = *this ^ *this;
        if (len2 <= 1e-20) return Vector3(0, 0, 0);
        return *this / std::sqrt(len2);
    }

    // Rodrigues' rotation
    inline Vector3 rotateAroundAxis(const Vector3 &a_unit, double ang) {
        const double c = std::cos(ang);
        const double s = std::sin(ang);
        return *this * c + (a_unit % *this) * s + a_unit * ((a_unit ^ *this) * (1.0 - c));
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

    friend Vector3 operator*(double left, const Vector3 &right) {
        return right * left;
    }

    // dot product
    double operator^(const Vector3 &right) const {
        return x * right.x + y * right.y + z * right.z;
    }

    // cross product
    Vector3 operator%(const Vector3 &right) const {
        return Vector3(
            y * right.z - z * right.y,
            z * right.x - x * right.z,
            x * right.y - y * right.x
        );
    }

    Vector3 operator!() const {
        return *this / this->length();
    }
};
