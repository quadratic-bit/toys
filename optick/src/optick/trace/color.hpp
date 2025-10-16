#pragma once
#include <cmath>

#include <SDL3/SDL_stdinc.h>

struct Color {
    double r, g, b; // 0..1 linear in sRGB

    Color() : r(0), g(0), b(0) {}
    Color(double rr, double gg, double bb) : r(rr), g(gg), b(bb) {}

    /**
     * Scale color by a scalar (brightness)
     */
    Color operator*(double scalar) const {
        return Color(r * scalar, g * scalar, b * scalar);
    }

    /**
     * Hadamard product of two colors, useful for tinting
     */
    Color operator*(const Color &other) const {
        return Color(r * other.r, g * other.g, b * other.b);
    }

    /**
     * Add colors (sum radiance)
     */
    Color operator+(const Color &other) const {
        return Color(r + other.r, g + other.g, b + other.b);
    }

    /**
     * Add color in-place (sum radiance)
     */
    Color &operator+=(const Color &other) {
        r += other.r;
        g += other.g;
        b += other.b;
        return *this;
    }

    /**
     * Convert x from linear [0,1] to gamma-encoded sRGB 8-bit.
     */
    static Uint8 encode(double x) {
        // clamp then gamma 2.2 (approximate sRGB gamma)
        if (x < 0) x = 0;
        if (x > 1) x = 1;
        return (Uint8)(std::pow(x, 1.0 / 2.2) * 255.0 + 0.5);
    }
};
