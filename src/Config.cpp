#include "Config.h"

#include <fstream>

Config* Config::s_instance = nullptr;

Config::Config() {
    load();
}

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
}

void Config::save() const {
    std::ofstream ofs("./NaratteMiru.conf", std::ios::trunc);
    if (!ofs)
        return;

    ofs << _path_lib  << "\n";
    ofs << _path_boot << "\n";
    ofs << _path_game << "\n";
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
