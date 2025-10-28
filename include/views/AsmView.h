#ifndef NARATTEMIRU_ASMVIEW_H
#define NARATTEMIRU_ASMVIEW_H

#include <cstdint>
#include <string>
#include <vector>

#include "Viewable.h"
#include "Naratte.h"

class AsmView : public Viewable {
public:
    explicit AsmView(Naratte* naratte);

    void render() override;
private:
    Naratte* _naratte{};
    long int _selected = -1;
};


#endif //NARATTEMIRU_ASMVIEW_H