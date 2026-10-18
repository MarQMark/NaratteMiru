#ifndef NARRATEMIRU_APPLICATION_H
#define NARRATEMIRU_APPLICATION_H
#include <memory>

#include "ui/View.h"


class Application {
public:
    Application();

    void update();

private:
    std::unique_ptr<View> _view{};
    std::unique_ptr<Window> _window{};
};


#endif //NARRATEMIRU_APPLICATION_H