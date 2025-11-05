#ifndef NARATTEMIRU_CONFIG_H
#define NARATTEMIRU_CONFIG_H
#include <string>


class Config {
private:
    Config();

    static Config* s_instance;
public:
    static Config* get();

    void load();
    void save() const;

    const std::string& getLibPath();
    const std::string& getBootPath();
    const std::string& getGamePath();

    void setLibPath(const std::string& path);
    void setBootPath(const std::string& path);
    void setGamePath(const std::string& path);

    int Ticks = 1000;
private:
    std::string _path_lib{};
    std::string _path_boot{};
    std::string _path_game{};
};


#endif //NARATTEMIRU_CONFIG_H