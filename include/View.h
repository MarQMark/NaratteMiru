#ifndef NARATTEMIRU_VIEW_H
#define NARATTEMIRU_VIEW_H

#include <map>
#include <string>

#include "views/Viewable.h"
#include "GLFW/glfw3.h"

class View {
public:
    View();
    ~View();

    void render();
    bool shouldRun() const;

    void addViewable(Viewable* viewable, const std::string& name);
    Viewable* getViewable(const std::string& name);

private:
    GLFWwindow* _window;
    std::map<std::string, Viewable*> _viewables;

    void render_dockspace();
};



#endif //NARATTEMIRU_VIEW_H
