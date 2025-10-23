#ifndef NARATTEMIRU_MIRU_H
#define NARATTEMIRU_MIRU_H
#include "View.h"
#include "GLFW/glfw3.h"


class Miru {
public:
    Miru();
    ~Miru();

    void update() const;

    bool shouldRun() const;

private:
    View* _view;
};


#endif //NARATTEMIRU_MIRU_H