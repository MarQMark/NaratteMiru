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

    bool getJPasCALL() const;
    void setJPasCALL(bool enabled);
    int getJaCStart() const;
    int getJaCEnd() const;
    void setJaCStart(int start);
    void setJaCEnd(int end);
    bool isJPasCall(int id, int max) const;

    int Ticks = 1000;

    bool Pause = false;

    bool dirtyCallStack();
private:
    std::string _path_lib{};
    std::string _path_boot{};
    std::string _path_game{};

    bool _jp_as_call = false;
    int _jac_start = 0;
    int _jac_end = -1;
    bool _dirty_cs = false;
};


#endif //NARATTEMIRU_CONFIG_H