#include "../trace/scene.hpp"

opt::Color MaterialEmissive::sample(TraceContext ctx) const {
    return ctx.target->color * Le;
}
