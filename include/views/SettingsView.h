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
    int _menu = 0;

    std::string _lib_path{};
    std::string _boot_path{};
    std::string _game_path{};

    void menu_general();
    void menu_format();

    void apply() const;
};


#endif //NARATTEMIRU_SETTINGSVIEW_H