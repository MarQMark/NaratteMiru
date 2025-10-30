#include "views/ViewportView.h"

#include "imgui.h"

ViewportView::ViewportView() {
    glGenTextures(1, &_fb_tex);
    glBindTexture(GL_TEXTURE_2D, _fb_tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    // allocate empty 256×256 RGBA buffer
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 256, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glBindTexture(GL_TEXTURE_2D, 0);
}

ViewportView::~ViewportView() {
    glDeleteTextures(1, &_fb_tex);
}

void ViewportView::render() {
    ImGui::Begin("Viewport");

    const float tex_width  = 256.0f;
    const float tex_height = 256.0f;
    const float tex_aspect = tex_width / tex_height;

    // available size inside the window
    ImVec2 avail = ImGui::GetContentRegionAvail();

    // scale while preserving aspect ratio
    float draw_width  = avail.x;
    float draw_height = avail.x / tex_aspect;
    if (draw_height > avail.y) {
        draw_height = avail.y;
        draw_width  = avail.y * tex_aspect;
    }

    draw_height = draw_width = 255*1;

    // compute centered position
    ImVec2 cursor = ImGui::GetCursorScreenPos();
    ImVec2 center_offset = ImVec2(
        (avail.x - draw_width) * 0.5f,
        (avail.y - draw_height) * 0.5f
    );

    // move cursor so image is centered
    ImGui::SetCursorScreenPos(ImVec2(cursor.x + center_offset.x,
                                     cursor.y + center_offset.y));

    ImGui::Image(_fb_tex,ImVec2(draw_width, draw_height));

    ImGui::End();
}

void ViewportView::updateFB(const uint32_t *fb) const {
    glBindTexture(GL_TEXTURE_2D, _fb_tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 256, 0,
                 GL_BGRA, GL_UNSIGNED_BYTE, fb);
    glBindTexture(GL_TEXTURE_2D, 0);
}
