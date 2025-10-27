#ifndef NARATTEMIRU_MENUBAR_H
#define NARATTEMIRU_MENUBAR_H

#include <string>
#include "views/Viewable.h"

class Menubar final : public Viewable{
public:
    void render() override;

    bool reload() const;
    void resetReload();

private:
    bool _asm_view = true;
    bool _memory_view = true;
    bool _register_view = true;
    bool _viewport_view = true;

    void add_menu_view(const std::string& name, bool* enabled) const;

    bool _reload = false;
};


#endif //NARATTEMIRU_MENUBAR_H