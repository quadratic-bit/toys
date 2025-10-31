#pragma once
#include <string>
#include <utility>
#include <vector>

#include "../trace/color.hpp"
#include "reflection.hpp"

using std::vector;
using std::pair;
using std::string;

// forward-declare
struct TraceContext;

class Material : public Reflectable {
public:
    Material() {}
    virtual ~Material() {}
    virtual Color sample(TraceContext ctx) const = 0;

    virtual bool  isEmissive() const { return false; }
    virtual Color emission()   const { return Color(0, 0, 0); }
};

// A mirror surface
class MaterialReflective : public Material {
public:
    MaterialReflective() : Material() {}

    Color sample(TraceContext ctx) const;

    void collectFields(FieldList &) {}
};

// A transparent material with a specified index of refraction (ior)
class MaterialRefractive : public Material {
public:
    double ior;

    MaterialRefractive(double ior_=1.5) : Material(), ior(ior_) {}

    Color sample(TraceContext ctx) const;

    void collectFields(FieldList &out) {
        Fields<MaterialRefractive>(this, out).add(&MaterialRefractive::ior, "ior");
    }
};

// An opaque surface with Lambert/Blinn-Phong lighting model
class MaterialOpaque : public Material {
public:
    double kd;         // diffuse albedo scale (0..1)
    double ks;         // specular weight for highlights (0..1)
    double shininess;  // Blinn-Phong exponent

    MaterialOpaque(double kd_=1.0, double ks_=0.0, double shininess_=32.0)
        : Material(), kd(kd_), ks(ks_), shininess(shininess_) {}

    Color sample(TraceContext ctx) const;

    void collectFields(FieldList &out) {
        Fields<MaterialOpaque>(this, out)
            .add(&MaterialOpaque::kd, "kd")
            .add(&MaterialOpaque::ks, "ks")
            .add(&MaterialOpaque::shininess, "shininess");
    }
};

class MaterialEmissive : public Material {
public:
    Color Le; // radiance emitted, e.g. 1..10

    explicit MaterialEmissive(const Color &Le_=Color(1, 1, 1)) : Le(Le_) {}

    Color sample(TraceContext ctx) const;
    bool  isEmissive() const { return true; }
    Color emission()   const { return Le; }

    void collectFields(FieldList &out) {
        Fields<MaterialEmissive>(this, out).add(&MaterialEmissive::Le, "Le");
    }
};
