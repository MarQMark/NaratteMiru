#ifndef NARATTEMIRU_TILEVIEW_H
#define NARATTEMIRU_TILEVIEW_H

#include "Naratte.h"
#include "Viewable.h"

class TileView : public Viewable{
public:
    explicit TileView(Naratte* naratte);

    void render() override;

private:
    Naratte* _naratte{};

    void render_tiles();
    void render_tm();
    void render_obj();
    void render_lcdc();

    void update_tile();

    int _selected = -1;
    int _tile = -1;
    uint8_t _tile_attr = 0;
    enum TILE_TYPE {
        BG,
        WIN,
        OBJ
    };

    TILE_TYPE _tile_type = OBJ;

    uint32_t _tile_txt = -1;

    int _bank = 0;
    int _addr_mode = 0;
};


#endif //NARATTEMIRU_TILEVIEW_H