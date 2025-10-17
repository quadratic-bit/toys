#pragma once
#include <limits>

#include "../geometry/vectors.hpp"
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
    static Ray primary(
            const Camera &cam, const CameraBasis &cb,
            int px, int py, int viewportW, int viewportH
    ) {
        // pixel centers (map to NDC [-1, 1])
        const double x_ndc = ( (px + 0.5) / (double)viewportW ) * 2.0 - 1.0;
        const double y_ndc = 1.0 - ( (py + 0.5) / (double)viewportH ) * 2.0;
        const double x_cam = x_ndc * cb.aspect * cb.tanHalfV;  // horizontal coordinate in camera space
        const double y_cam = y_ndc * cb.tanHalfV;              // vertical coordinate in camera space
        Vector3 dir = !(cb.fwd + x_cam * cb.right + y_cam * cb.up);  // forward + offsets, normalized
        return Ray(cam.pos, dir);
    }
};
