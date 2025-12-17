#include <sstream>
#include <string_view>
#include <unordered_map>
#include <dlfcn.h>

#include <cum/manager.hpp>
#include <cum/plugin.hpp>

namespace cum {

Plugin *Manager::LoadFromFile(const std::string_view path) {
    using std::string_view;

    void *so = dlopen(path.data(), RTLD_LAZY);
    if (!so)
        throw LoadError(dlerror());

    CreatePluginFn createPlugin = reinterpret_cast<CreatePluginFn>(
        dlsym(so, CreatePluginFuncNameStr.c_str())
    );

    if (!createPlugin) {
        dlclose(so);
        throw LoadError("CreatePlugin() function missing in the plugin");
    }

    Plugin *plugin = createPlugin();
    if (!plugin) {
        dlclose(so);
        throw LoadError("CreatePlugin() returned nullptr");
    }

    plugin->soHandle = so;
    plugin->manager = this;

    plugins.emplace_back(plugin);
    return plugin;
}

Plugin *Manager::GetById(std::string_view id) const {
    for (auto &p : plugins)
        if (p->GetIdentifier() == id)
            return p.get();
    return nullptr;
}

const std::vector<std::unique_ptr<Plugin>> &Manager::GetAll() const {
    return plugins;
}

void Manager::TriggerAfterLoad() {
    std::unordered_map<std::string_view, Plugin*> byId;
    byId.reserve(plugins.size());

    for (auto &p : plugins)
        byId[p->GetIdentifier()] = p.get();

    for (auto &p : plugins) {
        auto id  = p->GetIdentifier();
        auto name = p->GetName();

        for (auto dep : p->GetDependencies()) {
            if (!byId.count(dep)) {
                std::ostringstream err;
                err << "Plugin " << id << " (`" << name << "`) "
                    << "is missing a dependency " << dep;
                throw DependencyError(err.str());
            }
        }

        for (auto conf : p->GetConflicts()) {
            auto it = byId.find(conf);
            if (it != byId.end()) {
                std::ostringstream err;
                err << "Plugin " << id << " (`" << name << "`) "
                    << "is conflicting with " << conf << " (`"
                    << it->second->GetName() << "`)";
                throw DependencyError(err.str());
            }
        }
    }

    for (auto &p : plugins)
        p->AfterLoad();
}

} // namespace cum
