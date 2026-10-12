#ifndef NARRATEMIRU_APPLICATION_H
#define NARRATEMIRU_APPLICATION_H
#include <memory>

#include "ui/View.h"


class Application {
public:
    Application() = default;

    void update();

private:
    std::unique_ptr<View> _view{};
};


#endif //NARRATEMIRU_APPLICATION_H