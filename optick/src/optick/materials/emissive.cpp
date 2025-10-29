#include "../trace/scene.hpp"

Color MaterialEmissive::sample(TraceContext ctx) const {
    return ctx.target->color * Le;
}

vector<Property> MaterialEmissive::getProperties() const {
    vector<Property> v;
    v.push_back(Property("Le", Le.str()));
    return v;
}
