#include "../trace/scene.hpp"

Color MaterialOpaque::sample(TraceContext ctx) const {
    Color out(0, 0, 0);      // accumulated outgoing radiance
    Vector3 V = -ctx.ray.d;  // direction to camera

    for (size_t oi = 0; oi < ctx.scene->objects.size(); ++oi) {
        const Object *obj = ctx.scene->objects[oi];
        if (!obj->mat->isEmissive()) continue;

        const Color Le = obj->mat->emission();  // radiance color
        const double power = 100.0;

        Vector3 Lvec = obj->center - ctx.hit.pos;  // hit -> light
        double dist2 = Lvec ^ Lvec;
        double dist  = std::sqrt(dist2);
        Vector3 Ldir = Lvec / dist;

        if (ctx.scene->occludedTowards(ctx.hit.pos, Ldir, dist, ctx.eps, obj)) continue;

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
