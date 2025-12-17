#pragma once
#include <iomanip>
#include <limits>
#include <sstream>

#include "camera.hpp"
#include "trace/color.hpp"

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

inline std::string double2string(double v, int max_decimals = 6) {
    if (std::fabs(v) < std::pow(10.0, -max_decimals)) {
        v = 0.0;
    }

    if (v == 0.0) return "0";

    std::ostringstream s;
    s.imbue(std::locale::classic());
    s << std::fixed << std::setprecision(max_decimals) << v;

    std::string out = s.str();

    auto dot = out.find('.');
    if (dot != std::string::npos) {
        std::size_t end = out.size() - 1;
        while (end > dot && out[end] == '0') {
            --end;
        }
        if (end == dot) {
            --end;
        }
        out.erase(end + 1);
    }

    return out;
}

template<typename T>
std::string stringify(const T &v) {
    std::ostringstream os; os << v; return os.str();
}
template<>
inline std::string stringify<std::string>(const std::string &s) { return s; }

template<>
inline std::string stringify<bool>(const bool &b) { return b ? "true" : "false"; }

template<typename T>
T parse_from_string(const std::string &s) {
    std::istringstream is(s);
    T val;
    is >> val;
    if (!is) throw std::runtime_error("parse error");
    return val;
}

template<>
inline bool parse_from_string<bool>(const std::string &s) {
    if (s == "1" || s == "true"  || s == "True") return true;
    if (s == "0" || s == "false" || s == "False") return false;
    throw std::runtime_error("parse error bool");
}

template<>
inline opt::Color parse_from_string<opt::Color>(const std::string &s) {
    std::istringstream is(s);
    opt::Color c; is >> c;
    if (!is) throw std::runtime_error("parse error Color");
    return c;
}

template<>
inline std::string stringify<Vector3>(const Vector3 &v) {
    return double2string(v.x) + " " + double2string(v.y) + " " + double2string(v.z);
}

template<>
inline Vector3 parse_from_string<Vector3>(const std::string &s) {
    // "x y z" or "x, y, z" or "(x,y,z)"
    double x=0, y=0, z=0;
    const char *p = s.c_str();

    auto skip = [&]() {
        while (*p && (std::isspace((unsigned char)*p) || *p=='(' || *p==')' || *p==',')) ++p;
    };

    auto read = [&](double &out) {
        skip();
        char *end = nullptr;
        out = std::strtod(p, &end);
        if (!end || end == p) throw std::runtime_error("bad Vector3");
        p = end;
    };

    read(x); read(y); read(z);
    return Vector3(x, y, z);
}
