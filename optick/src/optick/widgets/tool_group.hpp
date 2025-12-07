#pragma once
#include <string>
#include <vector>

#include <pp/tool.hpp>

struct ToolGroupDesc {
    std::string label;             // plugin name
    std::string plugin_id;
    std::vector<pp::Tool*> tools;  // tools created by that plugin, in order
};
