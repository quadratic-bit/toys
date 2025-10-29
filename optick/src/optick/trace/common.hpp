#pragma once
#include <iomanip>
#include <limits>
#include <sstream>

#include "camera.hpp"

struct Hit {
    double  dist;     // distance along ray
    Vector3 pos;      // world-space hit position
    Vector3 norm;     // surface normal
    int     obj_i;    // which object was hit, -1 if none
    Hit() : dist(std::numeric_limits<double>::infinity()), obj_i(-1) {}
};

struct Ray {
    Vector3 o; // origin
    Vector3 d; // direction (normalized)

    Ray() {}
    Ray(const Vector3 &oo, const Vector3 &dd) : o(oo), d(!dd) {}

    /**
     * Primary ray through pixel center
     */
    static Ray primary(const Camera &cam, int px, int py, int viewportW, int viewportH) {
        // pixel centers (map to NDC [-1, 1])
        const double x_ndc = ( (px + 0.5) / (double)viewportW ) * 2.0 - 1.0;
        const double y_ndc = 1.0 - ( (py + 0.5) / (double)viewportH ) * 2.0;
        const double x_cam = x_ndc * cam.b.aspect * cam.b.tanHalfV;  // horizontal coordinate in camera space
        const double y_cam = y_ndc * cam.b.tanHalfV;                 // vertical coordinate in camera space
        Vector3 dir = !(cam.b.fwd + x_cam * cam.b.right + y_cam * cam.b.up);  // forward + offsets, normalized
        return Ray(cam.pos, dir);
    }
};

inline std::string double2string(double v, int precision=2) {
    std::ostringstream s;
    s << std::setprecision(precision) << v;
    return s.str();
}
