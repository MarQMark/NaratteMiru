#include "views/ViewportView.h"

#include "Config.h"
#include "imgui.h"

//#define VIEWPORT_WIDTH 256
//#define VIEWPORT_HEIGHT 256
#define VIEWPORT_WIDTH 160
#define VIEWPORT_HEIGHT 144

ViewportView::ViewportView(Naratte* naratte) : _naratte(naratte) {
    glGenTextures(1, &_fb_tex);
    glBindTexture(GL_TEXTURE_2D, _fb_tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    // allocate empty 256×256 RGBA buffer
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, VIEWPORT_WIDTH, VIEWPORT_HEIGHT, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glBindTexture(GL_TEXTURE_2D, 0);
}

ViewportView::~ViewportView() {
    glDeleteTextures(1, &_fb_tex);
}

void ViewportView::render() {
    if (_naratte->isDirty()) {
        updateFB(_naratte->getFB());
        _naratte->resetDirty();
    }

    ImGui::Begin("Viewport");

    const int buffer = _buffer;
    if (buffer == 0) ImGui::BeginDisabled();
    if (ImGui::Button("All"))  _buffer = 0;
    if (buffer == 0) ImGui::EndDisabled();
    ImGui::SameLine();
    if (buffer == 1) ImGui::BeginDisabled();
    if (ImGui::Button("BG"))  _buffer = 1;
    if (buffer == 1) ImGui::EndDisabled();
    ImGui::SameLine();
    if (buffer == 2) ImGui::BeginDisabled();
    if (ImGui::Button("Win"))  _buffer = 2;
    if (buffer == 2) ImGui::EndDisabled();
    ImGui::SameLine();
    if (buffer == 3) ImGui::BeginDisabled();
    if (ImGui::Button("Obj"))  _buffer = 3;
    if (buffer == 3) ImGui::EndDisabled();
    ImGui::SameLine();
    if (buffer == 4) ImGui::BeginDisabled();
    if (ImGui::Button("Prio"))  _buffer = 4;
    if (buffer == 4) ImGui::EndDisabled();

    ImGui::Separator();

    const float tex_width  = VIEWPORT_WIDTH;
    const float tex_height = VIEWPORT_HEIGHT;
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

    if (Config::get()->isViewportFixed()) {
        draw_width = VIEWPORT_WIDTH;
        draw_height = VIEWPORT_HEIGHT;
    }

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
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, VIEWPORT_WIDTH, VIEWPORT_HEIGHT, 0,
                 GL_BGRA, GL_UNSIGNED_BYTE, fb + 160 * 144 * _buffer);
    glBindTexture(GL_TEXTURE_2D, 0);
}
