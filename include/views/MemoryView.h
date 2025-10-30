#ifndef NARATTEMIRU_MEMORYVIEW_H
#define NARATTEMIRU_MEMORYVIEW_H

#include "Naratte.h"
#include "Viewable.h"

class MemoryView : public Viewable {
public:
    explicit MemoryView(Naratte* naratte);

    void render() override;
private:
    Naratte* _naratte{};
};


#endif //NARATTEMIRU_MEMORYVIEW_H