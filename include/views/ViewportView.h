#ifndef NARATTEMIRU_VIEWPORTVIEW_H
#define NARATTEMIRU_VIEWPORTVIEW_H

#include <cstdint>

#include "Viewable.h"

#define IMGUI_IMPL_OPENGL_LOADER_NONE
#include <GL/gl.h>

#include "Naratte.h"

class ViewportView : public Viewable{
public:
    explicit ViewportView(Naratte* naratte);
    ~ViewportView() override;

    void render() override;

    int getSelectedBuffer() const;

    void updateFB(const uint32_t* fb) const;

private:
    Naratte* _naratte{};
    GLuint _fb_tex = 0;

    int _buffer = 0; // 0: All, 1: BG, 2: Win, 3: Obj, 4: Prio
};


#endif //NARATTEMIRU_VIEWPORTVIEW_H