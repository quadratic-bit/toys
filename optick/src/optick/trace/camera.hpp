#pragma once
#include "../geometry/vectors.hpp"

struct Camera {
    Vector3 pos;     // world position
    Vector3 target;  // look-at point
    Vector3 up;
    double  vfov;    // vertical fov (in degrees)
    double  width;   // in pixels
    double  height;  // in pixels
};

struct CameraBasis {
    Vector3 fwd;       // unit
    Vector3 right;     // unit
    Vector3 up;        // unit
    double  tanHalfV;  // precomputed tan(verticalFov/2)
    double  aspect;    // width/height

    static CameraBasis make(const Camera &cam) {
        CameraBasis cb;

        cb.fwd = !(cam.target - cam.pos);
        cb.right = !(cb.fwd % cam.up);
        cb.up = cb.right % cb.fwd;

        const double vfov_rad = cam.vfov * (M_PI / 180.0);

        // Precompute tan(vFOV/2) for NDC->camera mapping
        cb.tanHalfV = std::tan(vfov_rad * 0.5);

        cb.aspect = cam.width / cam.height;
        return cb;
    }
};

inline bool project_point(const Camera &cam, const CameraBasis &cb,
                          int w, int h, const Vector3 &P, double eps,
                          int *sx, int *sy) {
    Vector3 d = P - cam.pos;
    double  x = d ^ cb.right;
    double  y = d ^ cb.up;
    double  z = d ^ cb.fwd;

    if (z <= eps) return false;

    double aspect = double(w) / double(h);
    double fy = 1.0 / std::tan((cam.vfov * M_PI / 180.0) * 0.5);
    double fx =  fy / aspect;

    double ndc_x = (x / z) * fx;
    double ndc_y = (y/z) * fy;

    *sx = int(std::floor(( ndc_x * 0.5 + 0.5) * w));
    *sy = int(std::floor((-ndc_y * 0.5 + 0.5) * h));
    return true;
}
