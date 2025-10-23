#include "scene.hpp"

Color MaterialReflective::sample(TraceContext ctx) const {
    Vector3 R = Vector3::reflect(ctx.ray.d, ctx.hit.norm);
    Color inc_clr = ctx.scene->trace(Ray(ctx.hit.pos + R * ctx.eps, R), ctx.depth + 1, ctx.max_depth, ctx.eps);
    // apply tint + faint base
    return ctx.target->color * inc_clr + Color(0.02, 0.02, 0.02);
}

Color MaterialRefractive::sample(TraceContext ctx) const {
    // clamped cosine between ray dir and normal
    double cos_incident = clampd(ctx.ray.d ^ ctx.hit.norm, -1.0, +1.0);

    double etai = 1.0;  // incident medium IoR
    double etat = ior;  // transmit medium IoR

    // FIXME: does this work for planes?
    Vector3 N = ctx.hit.norm;
    if (cos_incident > 0.0) {  // inside the sphere
        N = N * -1.0;
        double tmp = etai; // swap media IoR
        etai = etat;
        etat = tmp;
    }

    Vector3 T;  // refracted direction (i.e. transmitted dir, unit)
    if (T.refract(ctx.ray.d, N, etai, etat)) {
        Color inc_clr = ctx.scene->trace(Ray(ctx.hit.pos + T * ctx.eps, T), ctx.depth + 1, ctx.max_depth, ctx.eps);
        return ctx.target->color * inc_clr;
    }
    else // Total Internal Reflection
    {
        Vector3 R = Vector3::reflect(ctx.ray.d, ctx.hit.norm);
        Color inc_clr = ctx.scene->trace(Ray(ctx.hit.pos + R * ctx.eps, R), ctx.depth + 1, ctx.max_depth, ctx.eps);
        return ctx.target->color * inc_clr;
    }
}

Color MaterialOpaque::sample(TraceContext ctx) const {
    Color out(0, 0, 0);      // accumulated outgoing radiance
    Vector3 V = -ctx.ray.d;  // direction to camera

    for (size_t oi = 0; oi < ctx.scene->objects.size(); ++oi) {
        const Object *obj = ctx.scene->objects[oi];
        if (!obj->mat->is_emissive()) continue;

        const Color Le = obj->mat->emission();  // radiance color
        const double power = 100.0;

        Vector3 Lvec = obj->center - ctx.hit.pos;  // hit -> light
        double dist2 = Lvec ^ Lvec;
        double dist  = std::sqrt(dist2);
        Vector3 Ldir = Lvec / dist;

        if (ctx.scene->occluded_towards(ctx.hit.pos, Ldir, dist, ctx.eps, obj)) continue;

        // incoming radiance estimate with 1/(4πr^2) falloff
        Color Lradiance = Le * (power / (4.0 * M_PI * dist2));

        // lambertian diffuse (cosine term)
        double ndotl = std::max(0.0, ctx.hit.norm ^ Ldir);

        // add diffuse contribution (albedo * kd * cos) * light
        out += (ctx.target->color * (kd * ndotl)) * Lradiance;

        // Blinn-Phong specular
        if (ks > 0.0) {
            Vector3 H = !(Ldir + V);  // half-vector
            double ndoth = std::max(0.0, ctx.hit.norm ^ H);
            double spec  = std::pow(ndoth, shininess) * ks;
            out += Lradiance * spec; // colored by light, not by albedo
        }
    }

    // constant ambient fill to avoid pure black
    out += ctx.target->color * (0.02 * kd);

    return out;
}

Color MaterialEmissive::sample(TraceContext ctx) const {
    return ctx.target->color * Le;
}
