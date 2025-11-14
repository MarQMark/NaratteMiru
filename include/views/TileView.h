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

    int _bank = 0;
};


#endif //NARATTEMIRU_TILEVIEW_H