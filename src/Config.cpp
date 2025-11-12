#include "Config.h"

#include <chrono>
#include <filesystem>
#include <fstream>

Config::Config() {
    load();

    if (_auto_reload)
        _last_modified = last_modified(_path_lib);
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

    std::getline(ifs, _path_lib);
    std::getline(ifs, _path_boot);
    std::getline(ifs, _path_game);
    std::string var;
    std::getline(ifs, var);
    if(!var.empty()) {
        try {
            _auto_reload = std::stoi(var);
        } catch (...) {}
    }
    std::getline(ifs, var);
    if(!var.empty()) {
        try {
            _jp_as_call = std::stoi(var);
        } catch (...) {}
    }
    std::getline(ifs, var);
    if(!var.empty()) {
        try {
            _jac_start = std::stoi(var);
        } catch (...) {}
    }
    std::getline(ifs, var);
    if(!var.empty()) {
        try {
            _jac_end = std::stoi(var);
        } catch (...) {}
    }
    std::getline(ifs, var);
    if(!var.empty()) {
        try {
            _monitor = std::stoi(var);
        } catch (...) {}
    }
    std::getline(ifs, var);
    if(!var.empty()) {
        try {
            _viewport_fixed = std::stoi(var);
        } catch (...) {}
    }
}

void Config::save() const {
    std::ofstream ofs("./NaratteMiru.conf", std::ios::trunc);
    if (!ofs)
        return;

    ofs << _path_lib  << "\n";
    ofs << _path_boot << "\n";
    ofs << _path_game << "\n";
    ofs << _auto_reload << "\n";
    ofs << _jp_as_call << "\n";
    ofs << _jac_start << "\n";
    ofs << _jac_end << "\n";
    ofs << _monitor << "\n";
    ofs << _viewport_fixed << "\n";
}

void Config::setAutoReload(const bool enable) {
    _auto_reload = enable;

    if (enable)
        _last_modified = last_modified(_path_lib);
}

bool Config::getAutoReload() const {
    return _auto_reload;
}

bool Config::libNaratteChanged() {
    if (!_auto_reload)
        return false;

    if (const auto lastModified = last_modified(_path_lib); _last_modified != lastModified) {
        _last_modified = lastModified;
        return true;
    }

    return false;
}

const std::string & Config::getLibPath() {
    return _path_lib;
}

const std::string & Config::getBootPath() {
    return _path_boot;
}

const std::string & Config::getGamePath() {
    return _path_game;
}

void Config::setLibPath(const std::string &path) {
    _path_lib = path;
}

void Config::setBootPath(const std::string &path) {
    _path_boot = path;
}

void Config::setGamePath(const std::string &path) {
    _path_game = path;
}

bool Config::getJPasCALL() const {
    return _jp_as_call;
}

void Config::setJPasCALL(const bool enabled) {
    _jp_as_call = enabled;
    _dirty_cs = true;
}

int Config::getJaCStart() const {
    return _jac_start;
}

int Config::getJaCEnd() const {
    return _jac_end;
}

void Config::setJaCStart(const int start) {
    _jac_start = start;
    _dirty_cs = true;
}

void Config::setJaCEnd(const int end) {
    _jac_end = end;
    _dirty_cs = true;
}

bool Config::isJPasCall(const int id, const int max) const {
    return _jp_as_call && id >= _jac_start && id <= (_jac_end < 0 ? max : _jac_end);
}

bool Config::isMonitored() const {
    return _monitor;
}

void Config::setMonitored(const bool enable) {
    if (enable != _monitor) {
        _monitor = enable;
        save();
    }
    else {
        _monitor = enable;
    }
}

bool Config::isViewportFixed() const {
    return _viewport_fixed;
}

void Config::setViewportFixed(const bool fixed) {
    _viewport_fixed = fixed;
}

bool Config::dirtyCallStack() {
    if (_dirty_cs) {
        _dirty_cs = false;
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

    const auto sctp = std::chrono::clock_cast<std::chrono::system_clock>(ftime);
    return std::chrono::system_clock::to_time_t(sctp);
}
