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
    Basis   b;

    double minPitchDeg;
    double maxPitchDeg;

    Camera(Vector3 pos_, double vfov_, double w, double h)
            : pos(pos_)
            , target(pos + Vector3(0, 0, -1))
            , vfov(vfov_)
            , width(w)
            , height(h)
            , minPitchDeg(-89)
            , maxPitchDeg(89) { makeBasis(); }

    void makeBasis() {
        Vector3 up(0, 1, 0);
        b.fwd   = !(target - pos);
        b.right = !(b.fwd % up);
        b.up    = b.right % b.fwd;

        const double vfov_rad = vfov * (M_PI / 180.0);

        // for NDC->camera mapping
        b.tanHalfV = std::tan(vfov_rad * 0.5);

        b.aspect = width / height;
    }

    bool projectPoint(int w, int h, const Vector3 &P, double eps, int *sx, int *sy) const {
        Vector3 d = P - pos;
        double  x = d ^ b.right;
        double  y = d ^ b.up;
        double  z = d ^ b.fwd;

        if (z <= eps) return false;

        double aspect = double(w) / double(h);
        double fy = 1.0 / std::tan((vfov * M_PI / 180.0) * 0.5);
        double fx =  fy / aspect;

        double ndc_x = (x / z) * fx;
        double ndc_y = (y / z) * fy;

        *sx = int(std::floor(( ndc_x * 0.5 + 0.5) * w));
        *sy = int(std::floor((-ndc_y * 0.5 + 0.5) * h));
        return true;
    }

    double getCurrentPitch() const {
        const Vector3 fwd = !(this->target - this->pos);
        double y = fwd.y;
        if (y < -1.0) y = -1.0;
        else if (y > 1.0) y = 1.0;
        return std::asin(y) * (180.0 / M_PI);
    }


    void yaw(double degrees) {
        const Vector3 worldUp(0, 1, 0);
        Vector3 fwd = !(this->target - this->pos);

        if (std::fabs(fwd ^ worldUp) > 1.0 - 1e-8) {
            Vector3 right = (worldUp % fwd).normalizeClamp();
            if (right.x == 0 && right.y == 0 && right.z == 0) right = Vector3(1, 0, 0);
            fwd = (fwd + right * 1e-6).normalizeClamp();
        }

        Vector3 fwdRot = fwd.rotateAroundAxis(worldUp, deg2rad(degrees));
        this->target = this->pos + !fwdRot;
        this->makeBasis();
    }

    void pitch(double degrees) {
        const Vector3 world_up(0, 1, 0);
        Vector3 fwd = !(this->target - this->pos);

        double p0 = getCurrentPitch();
        double p1 = clamp(p0 + degrees, minPitchDeg, maxPitchDeg);
        double delta = p1 - p0;
        if (std::fabs(delta) < 1e-9) return;

        Vector3 right = (fwd % world_up).normalizeClamp();
        if (right.x == 0 && right.y == 0 && right.z == 0) {
            right = Vector3(1, 0, 0);
        }

        Vector3 fwd_rot = fwd.rotateAroundAxis(right, deg2rad(delta));
        fwd_rot = !fwd_rot;

        Vector3 right2 = (fwd_rot % world_up).normalizeClamp();
        if (right2.x == 0 && right2.y == 0 && right2.z == 0) {
            return;
        }

        this->target = this->pos + fwd_rot;
        this->makeBasis();
    }

    // positive forwards
    void move(double dist) {
        Vector3 fwd = !(this->target - this->pos);
        Vector3 fwd_xz = Vector3(fwd.x, 0.0, fwd.z).normalizeClamp();
        if (fwd_xz.x == 0 && fwd_xz.y == 0 && fwd_xz.z == 0) return;

        Vector3 delta = fwd_xz * dist;
        this->pos     = this->pos    + delta;
        this->target  = this->target + delta;
        this->makeBasis();
    }

    // positive right
    void strafe(double dist) {
        const Vector3 world_up(0, 1, 0);
        Vector3 fwd   = !(this->target - this->pos);
        Vector3 right = (fwd % world_up).normalizeClamp();
        if (right.x == 0 && right.y == 0 && right.z == 0) return;

        Vector3 delta = right * dist;
        this->pos     = this->pos    + delta;
        this->target  = this->target + delta;
        this->makeBasis();
    }

    // positive up
    void elevate(double dist) {
        const Vector3 world_up(0, 1, 0);
        Vector3 delta = world_up * dist;

        this->pos    = this->pos    + delta;
        this->target = this->target + delta;
        this->makeBasis();
    }
};
