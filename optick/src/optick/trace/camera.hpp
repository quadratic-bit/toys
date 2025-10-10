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
