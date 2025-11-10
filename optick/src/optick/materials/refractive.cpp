#include "../trace/scene.hpp"

opt::Color MaterialRefractive::sample(TraceContext ctx) const {
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
        opt::Color inc_clr = ctx.scene->trace(Ray(ctx.hit.pos + T * ctx.eps, T), ctx.depth + 1, ctx.max_depth, ctx.eps);
        return ctx.target->color * inc_clr;
    }
    else // Total Internal Reflection
    {
        Vector3 R = Vector3::reflect(ctx.ray.d, ctx.hit.norm);
        opt::Color inc_clr = ctx.scene->trace(Ray(ctx.hit.pos + R * ctx.eps, R), ctx.depth + 1, ctx.max_depth, ctx.eps);
        return ctx.target->color * inc_clr;
    }
}
