#include "Application.h"

Application::Application() {
    _window = std::make_unique<Window>(1920, 1080, "NarrateMiru");
    _view = std::make_unique<View>(*_window);
}

void Application::update() {
    while (!_window->shouldClose()) {
        _view->update();
        _window->update();
    }
}
