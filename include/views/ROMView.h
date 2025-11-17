#ifndef NARATTEMIRU_ROMVIEW_H
#define NARATTEMIRU_ROMVIEW_H

#include "Naratte.h"
#include "Viewable.h"

class ROMView : public  Viewable{
public:
    explicit ROMView(Naratte* naratte);

    void render() override;

private:
    Naratte* _naratte{};

    void load_roms();
    std::vector<std::string> _roms{};
    int _selected = -1;
};


#endif //NARATTEMIRU_ROMVIEW_H