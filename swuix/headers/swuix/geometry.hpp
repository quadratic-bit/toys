#include <algorithm>

#include <dr4/math/rect.hpp>

inline dr4::Rect2f intersect(const dr4::Rect2f &a, const dr4::Rect2f &b) {
    float left  = std::max(a.pos.x, b.pos.x);
    float top   = std::max(a.pos.y, b.pos.y);
    float right = std::min(a.pos.x + a.size.x, b.pos.x + b.size.x);
    float btm   = std::min(a.pos.y + a.size.y, b.pos.y + b.size.y);

    float cross_width = right - left;
    float cross_height = btm - top;
    if (cross_width <= 0.0 || cross_height <= 0.0) return dr4::Rect2f(0, 0, 0, 0);
    return dr4::Rect2f(left, top, cross_width, cross_height);
}

inline float clamp(float value, float lower, float upper) {
    return std::min(std::max(value, lower), upper);
}
