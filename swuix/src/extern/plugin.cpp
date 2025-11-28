#include "extern/plugin.hpp"
#include "cum/plugin.hpp"

extern "C" cum::Plugin *CreatePlugin();
extern "C" cum::Plugin *CreatePlugin() {
    return new SwuixBackend();
}
