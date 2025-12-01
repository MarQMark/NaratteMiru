#ifndef NARATTEMIRU_CONFIG_H
#define NARATTEMIRU_CONFIG_H
#include <cstdint>
#include <ctime>
#include <string>


class Config {
private:
    Config();

    static Config* s_instance;
public:
    static Config* get();

    void load();
    void save() const;

    void setAutoReload(bool enable);
    bool getAutoReload() const;
    bool libNaratteChanged();

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

    bool isMonitored() const;
    void setMonitored(bool enable);

    bool isEndlessLoop() const;
    void setEndlessLoop(bool enable);

    bool Pause = false;
    bool Reload = false;

    bool dirtyCallStack();

    uint8_t Joypad = 0;

private:
    bool _auto_reload = false;
    std::time_t _last_modified;
    std::string _path_lib{};
    std::string _path_boot{};
    std::string _path_game{};

    bool _jp_as_call = false;
    int _jac_start = 0;
    int _jac_end = -1;
    bool _dirty_cs = false;

    bool _monitor = true;
    bool _endless_loop = true;

    static std::time_t last_modified(const std::string& path);
};


#endif //NARATTEMIRU_CONFIG_H