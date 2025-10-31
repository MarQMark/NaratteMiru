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

    void updateFB(const uint32_t* fb) const;

private:
    Naratte* _naratte{};
    GLuint _fb_tex = 0;
};


#endif //NARATTEMIRU_VIEWPORTVIEW_H