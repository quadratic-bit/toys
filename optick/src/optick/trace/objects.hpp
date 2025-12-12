#pragma once
#include <array>
#include <cmath>
#include <vector>
#include <string>

#include "../materials/material.hpp"
#include "common.hpp"

using std::string;

struct AABB {
    Vector3 mn, mx;

    AABB() {
        static const double inf = std::numeric_limits<double>::infinity();
        mn = Vector3( inf,  inf,  inf);
        mx = Vector3(-inf, -inf, -inf);
    }

    void include(const Vector3 &p) {
        if (p.x < mn.x) mn.x = p.x;
        if (p.y < mn.y) mn.y = p.y;
        if (p.z < mn.z) mn.z = p.z;

        if (p.x > mx.x) mx.x = p.x;
        if (p.y > mx.y) mx.y = p.y;
        if (p.z > mx.z) mx.z = p.z;
    }

    void include(const AABB &b) {
        include(b.mn);
        include(b.mx);
    }

    bool isValid() const {
        return mn.x <= mx.x && mn.y <= mx.y && mn.z <= mx.z;
    }
};

class Object : public Reflectable {
    bool is_selected;

public:
    Vector3 center;
    opt::Color   color;
    string  name;

    Material *mat;

    Object(string name_, const Vector3 &pos, const opt::Color &col, Material *m)
        : is_selected(false), center(pos), color(col), name(name_), mat(m) {}

    virtual ~Object() {};

    bool selected() const {
        return is_selected;
    }

    bool toggleSelect() {
        return is_selected = !is_selected;
    }

    void select() {
        is_selected = true;
    }

    void unselect() {
        is_selected = false;
    }

    virtual bool intersect(const Ray &ray, double eps, Hit *hit) const = 0;

    // false = no finite box
    virtual bool worldAABB(AABB *out) const = 0;

    void collectFields(FieldList &out) override {
        Fields<Object>(this, out)
            .add(&Object::name,   "name")
            .add(&Object::center, "center")
            .add(&Object::color,  "color");
    }
};

struct Sphere : public Object {
    double radius;

    Sphere(string name_, const Vector3 &c, double r, const opt::Color &col, Material *m)
        : Object(name_, c, col, m), radius(r) {}

    /**
     * Ray-sphere intersection using stable quadratic form
     */
    bool intersect(const Ray &ray, double eps, Hit *hit) const override {
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

    bool worldAABB(AABB *out) const override {
        const Vector3 r(radius, radius, radius);
        out->mn = center - r;
        out->mx = center + r;
        return true;
    }

    void collectFields(FieldList &out) override {
        Object::collectFields(out);
        Fields<Sphere>(this, out).add(&Sphere::radius, "radius");
    }
};

struct Plane : public Object {
    Vector3 normal; // must be unit

    Plane(string name_,
          const Vector3  &point_on_plane,
          const Vector3  &n,
          const opt::Color    &col,
                Material *m)
        : Object(name_, point_on_plane, col, m), normal(!n) {}

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

    bool worldAABB(AABB *out) const {
        (void)out;
        return false;
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

    Polygon(string name_, const std::vector<Vector3> &verts, const opt::Color &col, Material *m)
            : Object(name_, Vector3(0, 0, 0), col, m),
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

    static bool pointInPolygon2D(const Vec2 &p, const std::vector<Vec2> &poly) {
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

        if (!pointInPolygon2D(p2, verts2)) return false;

        hit->pos  = p;
        hit->norm = (denom < 0.0) ? normal : (normal * -1.0);
        hit->dist = t;
        return true;
    }

    bool worldAABB(AABB *out) const {
        if (verts3.empty()) return false;

        AABB box;
        for (size_t i = 0; i < verts3.size(); ++i) box.include(verts3[i]);
        *out = box;

        return out->isValid();
    }
};

struct Tetrahedron : public Object {
    std::array<Vector3, 4> v;
    std::array<std::array<int,3>, 4> f;

    Tetrahedron(std::string name_,
                const Vector3 &v0,
                const Vector3 &v1,
                const Vector3 &v2,
                const Vector3 &v3,
                const opt::Color &col,
                Material *m)
        : Object(std::move(name_), Vector3(0,0,0), col, m)
        , v{v0, v1, v2, v3}
        , f{{
            {{0,1,2}},
            {{0,1,3}},
            {{0,2,3}},
            {{1,2,3}}
        }}
    {
        center = (v[0] + v[1] + v[2] + v[3]) * 0.25;
    }

    static bool intersectTri(const Ray &ray,
                             const Vector3 &a,
                             const Vector3 &b,
                             const Vector3 &c,
                             double eps,
                             double *tOut,
                             Vector3 *nOut)
    {
        // Moller–Trumbore
        Vector3 e1 = b - a;
        Vector3 e2 = c - a;

        Vector3 pvec = ray.d % e2;
        double det = e1 ^ pvec;

        if (std::fabs(det) < 1e-12) return false;
        double invDet = 1.0 / det;

        Vector3 tvec = ray.o - a;
        double u = (tvec ^ pvec) * invDet;
        if (u < 0.0 || u > 1.0) return false;

        Vector3 qvec = tvec % e1;
        double v = (ray.d ^ qvec) * invDet;
        if (v < 0.0 || u + v > 1.0) return false;

        double t = (e2 ^ qvec) * invDet;
        if (t <= eps) return false;

        Vector3 n = !(e1 % e2);
        if ((ray.d ^ n) > 0.0) n = n * -1.0;

        *tOut = t;
        *nOut = n;
        return true;
    }

    bool intersect(const Ray &ray, double eps, Hit *hit) const override {
        double bestT = 0.0;
        Vector3 bestN;
        bool any = false;

        for (const auto &fi : f) {
            const Vector3 &a = v[fi[0]];
            const Vector3 &b = v[fi[1]];
            const Vector3 &c = v[fi[2]];

            double t = 0.0;
            Vector3 n;
            if (!intersectTri(ray, a, b, c, eps, &t, &n)) continue;

            if (!any || t < bestT) {
                any = true;
                bestT = t;
                bestN = n;
            }
        }

        if (!any) return false;

        hit->pos  = ray.o + ray.d * bestT;
        hit->norm = bestN;
        hit->dist = bestT;
        return true;
    }

    bool worldAABB(AABB *out) const override {
        AABB box;
        for (int i = 0; i < 4; ++i) box.include(v[i]);
        *out = box;
        return out->isValid();
    }
};
