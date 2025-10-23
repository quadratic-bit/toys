#pragma once
#include <cmath>

// 2D viewport placement with explicit axes in pixels-per-unit.
struct ViewCS {
    // pixel-space origin (top-left of render area) in window pixels
    int ox, oy;

    // screen-space axes in pixels per image unit
    double axx, axy; // where +X steps move in pixels
    double ayx, ayy; // where +Y steps move in pixels

    ViewCS() : ox(0), oy(0), axx(1), axy(0), ayx(0), ayy(1) {}
    ViewCS(int origin_x, int origin_y, double xaxX, double xaxY, double yaxX, double yaxY)
        : ox(origin_x), oy(origin_y), axx(xaxX), axy(xaxY), ayx(yaxX), ayy(yaxY) {}

    /**
     * Map an image sample (ix, iy) to window pixel (px, py)
     */
    inline void toPixels(int ix, int iy, int *px, int *py) const {
        *px = ox + static_cast<int>(std::floor(ix * axx + iy * ayx + 0.5));
        *py = oy + static_cast<int>(std::floor(ix * axy + iy * ayy + 0.5));
    }
};
