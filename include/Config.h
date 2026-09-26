#ifndef NARATTEMIRU_CONFIG_H
#define NARATTEMIRU_CONFIG_H
#include <cstdint>
#include <ctime>
#include <string>

#include "nlohmann/json.hpp"

class Config {
private:
    Config();

    static Config* s_instance;
public:
    static Config* get();

    void load();
    void save() const;

private:
    template<typename T>
    class Tracked {
    public:
        using Setter = std::function<void(const T&)>;

        explicit Tracked(bool& changed, T value = {}, Setter setter = {})
            : _value(value), _changed(changed), _setter(setter) {}

        Tracked& operator=(const T& value) {
            if (_value != value) {
                _value = value;
                _changed = true;
            }
            if (_setter)
                _setter(value);
            return *this;
        }

        operator const T&() const {
            return _value;
        }
        const T& get() const {
            return _value;
        }

        void load(const T& value) {
            _value = value;
        }

        friend void to_json(nlohmann::json& j, const Tracked& value) {
            j = value._value;
        }

        friend void from_json(const nlohmann::json& j, Tracked& value) {
            value.load(j.get<T>());
        }


    private:
        T _value;
        bool& _changed;
        Setter _setter;
    };

public:
    struct {
        bool changed = false;

        Tracked<bool> autoReload{changed, false,
            [&](const bool enabled) {
                if (enabled)
                    get()->_last_modified = last_modified(get()->settings.pathLib.get());
            }
        };
        Tracked<std::string> pathLib{changed};
        Tracked<std::string> pathBoot{changed};
        Tracked<std::string> pathRom{changed};

        Tracked<bool> stopInfLoop{changed, false};
    } settings;

    struct {
        bool changed = false;

        Tracked<bool> monitoring{changed, false};

    } properties;

    bool libNaratteChanged();

    int Ticks = 1000;

    bool Pause = false;
    bool Reload = false;

    uint8_t Joypad = 0;

private:
    std::time_t _last_modified;
    static std::time_t last_modified(const std::string& path);
};


#endif //NARATTEMIRU_CONFIG_H