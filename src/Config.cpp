#include "Config.h"

#include <chrono>
#include <filesystem>
#include <fstream>

Config::Config() {
    load();

    if (settings.autoReload)
        _last_modified = last_modified(settings.pathLib.get());
}

Config* Config::s_instance = nullptr;

Config* Config::get() {
    if (!s_instance)
        s_instance = new Config();

    return s_instance;
}

void Config::load() {
    std::ifstream ifs("./NaratteMiru.conf");
    if (!ifs)
        return;

    try {
        nlohmann::json j;
        ifs >> j;

        if (!j.is_object())
            return;

        if (j.contains("Settings") && j["Settings"].is_object()) {
            const auto& s = j["Settings"];

            if (s.contains("autoReload") && s["autoReload"].is_boolean())
                settings.autoReload.load(s["autoReload"].get<bool>());
            if (s.contains("pathLib") && s["pathLib"].is_string())
                settings.pathLib.load(s["pathLib"].get<std::string>());
            if (s.contains("pathBoot") && s["pathBoot"].is_string())
                settings.pathBoot.load(s["pathBoot"].get<std::string>());
            if (s.contains("pathRom") && s["pathRom"].is_string())
                settings.pathRom.load(s["pathRom"].get<std::string>());
            if (s.contains("stopInfLoop") && s["stopInfLoop"].is_boolean())
                settings.stopInfLoop.load(s["stopInfLoop"].get<bool>());
        }

        if (j.contains("Properties") && j["Properties"].is_object()) {
            const auto& p = j["Properties"];

            if (p.contains("monitoring") && p["monitoring"].is_boolean())
                properties.monitoring.load(p["monitoring"].get<bool>());

            if (p.contains("viewsVisible") && p["viewsVisible"].is_object()) {
                for (const auto& [name, value] : p["viewsVisible"].items()) {
                    if (value.is_boolean()) {
                        properties.viewsVisible.emplace(
                            name,
                            Tracked<bool>{properties.changed, value.get<bool>()}
                        );
                    }
                }
            }
        }
    }
    catch (const nlohmann::json::exception&) {
        // Invalid JSON or an unexpected JSON error.
        return;
    }
}

void Config::save() const {
    std::ofstream ofs("./NaratteMiru.conf", std::ios::trunc);
    if (!ofs)
        return;

    nlohmann::json j;
    j["Settings"] = {
        {"autoReload", settings.autoReload},
        {"pathLib", settings.pathLib},
        {"pathBoot", settings.pathBoot},
        {"pathRom", settings.pathRom},
        {"stopInfLoop", settings.stopInfLoop}
    };

    j["Properties"] = {
        {"monitoring", properties.monitoring},
        {"viewsVisible", properties.viewsVisible}
    };

    ofs << j.dump(4);
}

bool Config::libNaratteChanged() {
    if (!settings.autoReload)
        return false;

    if (const auto lastModified = last_modified(settings.pathLib.get()); _last_modified != lastModified) {
        _last_modified = lastModified;
        return true;
    }

    return false;
}

std::time_t Config::last_modified(const std::string &path) {
    namespace fs = std::filesystem;
    std::error_code ec;
    const auto ftime = fs::last_write_time(path, ec);
    if (ec)
        return -1;

    const auto sctp = std::chrono::file_clock::to_sys(ftime);
    return std::chrono::system_clock::to_time_t(sctp);
}
