#include <cum/ifc/pp.hpp>
#include <cum/plugin.hpp>

#include "./pp_line_tool.cpp"
#include "./pp_text_tool.cpp"

class PrimitivesPlugin : public cum::PPToolPlugin {
public:
    std::string_view GetIdentifier() const override {
        return "pp.tools.essentials";
    }

    std::string_view GetName() const override {
        return "Essential tools";
    }

    std::string_view GetDescription() const override {
        return "Essential tools for pp::Canvas";
    }

    std::vector<std::string_view> GetDependencies() const override { return {}; }
    std::vector<std::string_view> GetConflicts() const override { return {}; }

    void AfterLoad() override {}

    std::vector<std::unique_ptr<pp::Tool>> CreateTools(pp::Canvas *cvs) override {
        std::vector<std::unique_ptr<pp::Tool>> tools;
        tools.emplace_back(std::make_unique<LineTool>(cvs));
        tools.emplace_back(std::make_unique<TextTool>(cvs));
        return tools;
    }
};

extern "C" cum::Plugin *CreatePlugin();
extern "C" cum::Plugin *CreatePlugin() {
    return new PrimitivesPlugin();
}
