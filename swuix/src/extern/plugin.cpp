#include "extern/plugin.hpp"

extern "C" dr4::DR4Backend *CreateDR4Backend(void) {
    return new SwuixBackend();
}
