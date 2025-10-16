#pragma once
#include <algorithm>
#include <cassert>

#include <swuix/common.hpp>

inline Rect2F frect(float x, float y, float w, float h) {
    Rect2F r;
    r.x = x;
    r.y = y;
    r.w = w;
    r.h = h;
    return r;
}

inline Rect2F intersect(const Rect2F &a, const Rect2F &b) {
    float left  = std::max(a.x, b.x);
    float top   = std::max(a.y, b.y);
    float right = std::min(a.x + a.w, b.x + b.w);
    float btm   = std::min(a.y + a.h, b.y + b.h);

    float cross_width = right - left;
    float cross_height = btm - top;
    if (cross_width <= 0.0 || cross_height <= 0.0) return frect(0, 0, 0, 0);
    return frect(left, top, cross_width, cross_height);
}

inline float clamp(float value, float lower, float upper) {
    return std::min(std::max(value, lower), upper);
}

class Vec2F {
public:
    float x;
    float y;

    Vec2F() : x(0), y(0) {}  // degenerate

    explicit Vec2F(float xx, float yy)
        : x(xx), y(yy) {}

    Vec2F operator-() const {
        return Vec2F(-x, -y);
    }

    Vec2F operator-(const Vec2F &other) const {
        return Vec2F(x - other.x, y - other.y);
    }

    Vec2F operator+(const Vec2F &other) const {
        return Vec2F(x + other.x, y + other.y);
    }

    void operator+=(const Vec2F &other) {
        x += other.x;
        y += other.y;
    }

    void operator-=(const Vec2F &other) {
        x -= other.x;
        y -= other.y;
    }

    bool operator==(const Vec2F &other) {
        return x == other.x && y == other.y;
    }
};
