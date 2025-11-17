#ifndef NARATTEMIRU_MENUBAR_H
#define NARATTEMIRU_MENUBAR_H

#include <string>

#include "Naratte.h"
#include "views/Viewable.h"

class Menubar final : public Viewable{
public:
    explicit Menubar(Naratte* naratte);

    void render() override;

private:
    Naratte* _naratte{};

    bool _asm_view = true;
    bool _memory_view = true;
    bool _register_view = true;
    bool _viewport_view = true;
    bool _tile_view = true;
    bool _rom_view = true;

    void add_menu_view(const std::string& name, bool* enabled) const;
};


#endif //NARATTEMIRU_MENUBAR_H