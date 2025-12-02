#ifndef NARATTEMIRU_SETTINGSVIEW_H
#define NARATTEMIRU_SETTINGSVIEW_H

#include <string>

#include "Viewable.h"

class SettingsView : public Viewable{
public:
    SettingsView();

    void render() override;

    void setVisible(const bool visible) override;

private:
    bool _dirty = false;

    int _menu = 0;

    std::string _lib_path{};
    bool _auto_reload = false;
    std::string _boot_path{};
    std::string _game_path{};

    bool _endless_loop = true;
    float _min_fr = 60;
    double _speed_multi = 1;

    bool _jp_as_call = false;
    int _jac_start = 0;
    int _jac_end = -1;

    void menu_general();
    void menu_emulation();
    void menu_format();

    void apply();
};


#endif //NARATTEMIRU_SETTINGSVIEW_H