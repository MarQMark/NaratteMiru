#ifndef NARATTEMIRU_MIRU_H
#define NARATTEMIRU_MIRU_H

#include "View.h"
#include "views/Menubar.h"
#include "views/AsmView.h"
#include "views/MemoryView.h"
#include "views/RegisterView.h"
#include "views/ViewportView.h"

class Miru {
public:
    Miru();
    ~Miru();

    void update();

    bool shouldRun() const;

private:
    View* _view{};
    Menubar* _menubar{};
    AsmView* _asm_view{};
    MemoryView* _memory_view{};
    RegisterView* _register_view{};
    ViewportView* _viewport_view{};

    void* _lib_handle{};
    void reload_lib();
};


#endif //NARATTEMIRU_MIRU_H