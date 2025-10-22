#pragma once
#include "../geometry/vectors.hpp"
#include "color.hpp"
#include "material.hpp"
#include "common.hpp"
#include <vector>

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

struct Polygon : public Object {
    std::vector<Vector3> verts3;
    Vector3 normal; // unit

    // precomputed local 2D basis
    struct Vec2 {
        double x, y;
        Vec2() : x(0), y(0) {}
        Vec2(double X, double Y) : x(X), y(Y) {}
    };
    Vector3 u, v;  // orthonormal (u,v,normal)
    std::vector<Vec2> verts2;  // projected onto (u,v)

    Polygon(const std::vector<Vector3> &verts, const Color &col, const Material *m)
            : Object(Vector3(0, 0, 0), col, m),
            verts3(verts),
            normal(0, 1, 0),
            u(1, 0, 0), v(0, 1, 0)
    {
        if (verts3.size() < 3) return;

        Vector3 c = Vector3();
        for (size_t i = 0; i < verts3.size(); ++i)
            c = c + verts3[i];

        center = c * (1.0 / double(verts3.size()));

        // Newell's
        Vector3 n = Vector3();
        for (size_t i = 0, nverts = verts3.size(); i < nverts; ++i) {
            const Vector3 &a = verts3[i];
            const Vector3 &b = verts3[(i + 1) % nverts];
            n.x += (a.y - b.y) * (a.z + b.z);
            n.y += (a.z - b.z) * (a.x + b.x);
            n.z += (a.x - b.x) * (a.y + b.y);
        }
        if ((n ^ n) > 0.0)
            normal = !n;

        Vector3 t = std::fabs(normal.z) < 0.999 ? Vector3(0, 0, 1) : Vector3(0, 1, 0);
        u = !(t % normal);
        v = normal % u;

        // precompute projections
        verts2.resize(verts3.size());
        for (size_t i = 0; i < verts3.size(); ++i) {
            Vector3 d = verts3[i] - center;
            verts2[i] = Vec2(d ^ u, d ^ v);
        }
    }

    static bool point_in_polygon_2d(const Vec2 &p, const std::vector<Vec2> &poly) {
        bool inside = false;
        const size_t n = poly.size();
        for (size_t i = 0, j = n - 1; i < n; j = i++) {
            const Vec2 &a = poly[j];
            const Vec2 &b = poly[i];

            // check if edge (a,b) straddles the horizontal ray at p.y
            const bool cond = ((a.y > p.y) != (b.y > p.y));
            if (cond) {
                double t = (p.y - a.y) / (b.y - a.y);
                double xint = a.x + t * (b.x - a.x);
                if (xint >= p.x) inside = !inside;
            }
        }

        if (inside) return true;

        // edge/vertex hit check
        for (size_t i = 0, j = n - 1; i < n; j = i++) {
            const Vec2 &a = poly[j];
            const Vec2 &b = poly[i];

            double abx = b.x - a.x, aby = b.y - a.y;
            double apx = p.x - a.x, apy = p.y - a.y;
            double cross = abx * apy - aby * apx;

            if (std::fabs(cross) < 1e-12) {
                double dot = apx * abx + apy * aby;
                double len2 = abx * abx + aby * aby;
                if (dot >= -1e-12 && dot <= len2 + 1e-12) return true;
            }
        }
        return false;
    }

    bool intersect(const Ray &ray, double eps, Hit *hit) const {
        if (verts3.size() < 3) return false;

        double denom = ray.d ^ normal;
        if (std::fabs(denom) < 1e-12) return false; // parallel

        // any point on plane; use center
        double t = ((center - ray.o) ^ normal) / denom;
        if (t <= eps) return false;

        Vector3 p = ray.o + ray.d * t;

        Vector3 d = p - center;
        Vec2 p2(d ^ u, d ^ v);

        if (!point_in_polygon_2d(p2, verts2)) return false;

        hit->pos  = p;
        hit->norm = (denom < 0.0) ? normal : (normal * -1.0);
        hit->dist = t;
        return true;
    }
};
