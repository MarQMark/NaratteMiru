#ifndef NARRATEMIRU_VIEW_H
#define NARRATEMIRU_VIEW_H
#include "Window.h"


class View {
public:
    explicit View(const Window& window);
    ~View() = default;

    void update();

private:
    void render_menubar();
};


#endif //NARRATEMIRU_VIEW_H