#include "Miru.h"

Miru::Miru() {
    _view = new View;
}

Miru::~Miru() {
    delete _view;
}

void Miru::update() const {
    _view->render();
}

bool Miru::shouldRun() const {
    return _view->shouldRun();
}
