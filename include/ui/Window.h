#ifndef NARRATEMIRU_WINDOW_H
#define NARRATEMIRU_WINDOW_H

#include <string>
#include <GLFW/glfw3.h>

class Window {
public:
    Window(int width, int height, std::string title);
    ~Window();

    bool shouldClose() const;
    void update() const;

    GLFWwindow *getWin() const;
private:
    int _width;
    int _height;
    std::string _title;
    GLFWwindow* _win{};
};


#endif //NARRATEMIRU_WINDOW_H