#ifndef NARATTEMIRU_VIEW_H
#define NARATTEMIRU_VIEW_H

#include <vector>

#include "Viewable.h"
#include "GLFW/glfw3.h"

class View {
public:
    View();
    ~View();

    void render();
    bool shouldRun() const;

    void addViewable(Viewable* viewable);

private:
    GLFWwindow* _window;
    std::vector<Viewable*> _viewables;

    void render_dockspace();
};



#endif //NARATTEMIRU_VIEW_H
