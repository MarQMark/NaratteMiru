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

    void add_menu_view(const std::string& name) const;
};


#endif //NARATTEMIRU_MENUBAR_H