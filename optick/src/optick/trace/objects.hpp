#pragma once
#include "../geometry/vectors.hpp"
#include "color.hpp"
#include "material.hpp"
#include "common.hpp"

struct PointLight {
    Vector3 pos;   // world
    Color   color;
    double  power; // scalar intensity, ~10..100

    PointLight() : color(1, 1, 1), power(10.0) {}
    PointLight(const Vector3 &p, const Color &c, double pw)
        : pos(p), color(c), power(pw) {}
};

struct Object {
    Vector3 center;
    Color   color;

    const Material *mat;

    Object(const Material *mat_) : mat(mat_) {}
    Object(const Vector3 &pos, const Color &col, const Material *m)
        : center(pos), color(col), mat(m) {}

    virtual ~Object() {};

    virtual bool intersect(const Ray &ray, double eps, Hit *hit) const = 0;
};

struct Sphere : public Object {
    double radius;

    Sphere(const Material *m) : Object(m), radius(1.0) {}

    Sphere(const Vector3 &c, double r, const Color &col, const Material *m)
        : Object(c, col, m), radius(r) {}

    /**
     * Ray-sphere intersection using stable quadratic form
     */
    bool intersect(const Ray &ray, double eps, Hit *hit) const {
        // Solve |o + t d - c|^2 = R^2  with a stable quadratic
        // d is normalized => a = 1
        Vector3 L = ray.o - this->center;
        const double b = 2.0 * (ray.d ^ L);
        const double c = (L ^ L) - this->radius * this->radius;
        const double disc = b * b - 4.0 * c;
        if (disc < 0.0) return false;

        const double sqrt_disc = std::sqrt(disc);
        // stable form
        double q = -0.5 * (b + (b >= 0 ? sqrt_disc : -sqrt_disc));
        double t0 = q;  // / a (a=1)
        double t1 = c / q;

        if (t0 > t1) {  // ensure t0 is the smaller root
            double tmp = t0;
            t0 = t1;
            t1 = tmp;
        }

        double t = t0;
        if (t <= eps) t = t1;  // root is too close or behind
        if (t <= eps) return false;  // both roots are bad

        hit->pos = ray.o + ray.d * t;
        hit->norm = !(hit->pos - this->center);  // outward (normalized)
        hit->dist = t;
        return true;
    }
};

struct Plane : public Object {
    Vector3 normal; // must be unit

    Plane(const Material *m) : Object(m), normal(0, 1, 0) {}

    Plane(const Vector3  &point_on_plane,
          const Vector3  &n,
          const Color    &col,
          const Material *m)
        : Object(point_on_plane, col, m), normal(!n) {}

    bool intersect(const Ray &ray, double eps, Hit *hit) const {
        const double denom = ray.d ^ normal;

        if (std::fabs(denom) < 1e-12) return false;  // ray parallel to plane

        // solve for t
        const double t = ((this->center - ray.o) ^ normal) / denom;
        if (t <= eps) return false;  // behind or too close

        hit->pos  = ray.o + ray.d * t;

        // orient the geometric normal
        hit->norm = (denom < 0.0) ? normal : (normal * -1.0);
        hit->dist = t;
        return true;
    }
};
