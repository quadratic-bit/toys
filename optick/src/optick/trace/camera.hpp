#pragma once
#include "../geometry/vectors.hpp"

struct Camera {

    struct Basis {
        Vector3 fwd;       // unit
        Vector3 right;     // unit
        Vector3 up;        // unit
        double  tanHalfV;  // precomputed tan(verticalFov/2)
        double  aspect;    // width/height
    };

    Vector3 pos;     // world position
    Vector3 target;  // look-at point
    double  vfov;    // vertical fov (in degrees)
    double  width;   // in pixels
    double  height;  // in pixels
    Basis b;

    Camera(Vector3 pos_, double vfov_, double w, double h) 
            : pos(pos_), target(pos + Vector3(0, 0, -1)), vfov(vfov_), width(w), height(h) {
        makeBasis();
    }

    void makeBasis() {
        Vector3 up(0, 1, 0);
        b.fwd = !(target - pos);
        b.right = !(b.fwd % up);
        b.up = b.right % b.fwd;

        const double vfov_rad = vfov * (M_PI / 180.0);

        // for NDC->camera mapping
        b.tanHalfV = std::tan(vfov_rad * 0.5);

        b.aspect = width / height;
    }

    bool project_point(int w, int h, const Vector3 &P, double eps, int *sx, int *sy) const {
        Vector3 d = P - pos;
        double  x = d ^ b.right;
        double  y = d ^ b.up;
        double  z = d ^ b.fwd;

        if (z <= eps) return false;

        double aspect = double(w) / double(h);
        double fy = 1.0 / std::tan((vfov * M_PI / 180.0) * 0.5);
        double fx =  fy / aspect;

        double ndc_x = (x / z) * fx;
        double ndc_y = (y/z) * fy;

        *sx = int(std::floor(( ndc_x * 0.5 + 0.5) * w));
        *sy = int(std::floor((-ndc_y * 0.5 + 0.5) * h));
        return true;
    }
};
