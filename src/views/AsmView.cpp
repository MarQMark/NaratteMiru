#include "views/AsmView.h"

#include <sstream>

#include "imgui.h"
#include "imgui_internal.h"
#include "misc/cpp/imgui_stdlib.h"

AsmView::AsmView(Naratte* naratte) : _naratte(naratte){
}

void AsmView::render() {

    if (_selected > _naratte->getInstructions().size())
        _selected = -1;

    ImGui::Begin("Assembly");


    ImGui::Text("Jump to:");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(150);
    ImGui::InputText("##jump_filter", &_jump_filter);

    ImGui::SameLine();
    if (ImGui::Button("Prev")) {
        jump_filter(false);
    }
    ImGui::SameLine();
    if (ImGui::Button("Next")) {
        jump_filter(true);
    }
    ImGui::SameLine();
    ImGui::Checkbox("Format", &_format);

    const ImGuiIO& io = ImGui::GetIO();
    const bool ctrl = io.KeyCtrl;
    // Ctrl + Up → prev
    if (ctrl && ImGui::IsKeyPressed(ImGuiKey_UpArrow))
        jump_filter(false);
    // Ctrl + Down → next
    if (ctrl && ImGui::IsKeyPressed(ImGuiKey_DownArrow))
        jump_filter(true);

    ImGui::Separator();

    if (!_format)
        render_raw();
    else
        render_format();

    ImGui::End();

    _naratte->setSelected(_selected);

    ImGui::Begin("Mem Writes");

    if (ImGui::BeginListBox("##instr_lis2t", ImVec2(-FLT_MIN, -FLT_MIN))) {
        for (const auto& [idx, addr, data] : _naratte->getMemWrites()) {
            if (_selected != idx)
                continue;

            ImGui::Text( "Addr: %04X,  Data: %02X", addr, data);
        }

        ImGui::EndListBox();
    }

    ImGui::End();
}

void AsmView::get_label(char *label, const size_t len, const int id) const {
    auto& instruction = _naratte->getInstructions()[id];
    char* name = _naratte->getInstructionName(instruction.op);

    char op1[4], op2[4];
    snprintf(op1, sizeof(op1), "%02X", instruction.op[0]);

    if (instruction.op[3] >= 2)
        snprintf(op2, sizeof(op2), "%02X", instruction.op[1]);
    else
        snprintf(op2, sizeof(op2), "  ");

    char op3[4];
    if (instruction.op[3] >= 3)
        snprintf(op3, sizeof(op3), "%02X", instruction.op[2]);
    else
        snprintf(op3, sizeof(op3), "  ");

    snprintf(label, len, "%06d | %s %s %s | %s",
             id, op1, op2, op3, name);


    if (instruction.isCall() || instruction.isJP()) {
        uint16_t addr = (instruction.op[2] << 8) | instruction.op[1];
        if (instruction.op[0] == 0xE9)
            addr = instruction.cpu.HL;
        sprintf(label + 40, "| %s", _naratte->getCallLabel(addr).c_str());
        for (int c = 39; label[c] == 0 && c >= 0; c--)
            label[c] = ' ';
    }
}

void AsmView::render_raw() {
    if (ImGui::BeginListBox("##instr_list", ImVec2(-FLT_MIN, -FLT_MIN))) {
        const float item_h = ImGui::GetTextLineHeightWithSpacing();

        if (_jump_to && _selected >= 0) {
            // Compute desired scroll Y in pixels so selected item is centered
            ImGuiWindow* listWindow = ImGui::GetCurrentWindow(); // this is the listbox child window
            float list_h = ImGui::GetWindowHeight();

            // If your visual rows differ from indices (e.g. grouped/expanded rows),
            // replace `_selected` with the visual-row index:
            int visualIndex = _selected; // <- change this if you use visibleRows

            float target_y = visualIndex * item_h - (list_h * 0.5f) + (item_h * 0.5f);
            if (target_y < 0.0f) target_y = 0.0f;

            ImGui::SetScrollY(target_y);   // set the child window scroll BEFORE the clipper
            _jump_to = false;    // done
        }

        ImGuiListClipper clipper;
        clipper.Begin(static_cast<int>(_naratte->getInstructions().size()));

        while (clipper.Step()) {
            for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; ++i) {
                const bool is_selected = (_selected == i);
                char label[128] = {};
                get_label(label, 128, i);

                if (ImGui::Selectable(label, is_selected))
                    _selected = i;
                if (ImGui::BeginPopupContextItem()) {
                    if (ImGui::MenuItem("Copy")) {
                        ImGui::SetClipboardText(std::to_string(i).c_str());
                    }
                    ImGui::EndPopup();
                }
                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }
        }

        ImGui::EndListBox();
    }
}

void AsmView::render_format() {
    if (ImGui::BeginListBox("##instr_list", ImVec2(-FLT_MIN, -FLT_MIN))) {
        int stackSize = 0;

        const float item_h = ImGui::GetTextLineHeightWithSpacing();
        /*if (_jump_to && _selected >= 0) {
            // Compute desired scroll Y in pixels so selected item is centered
            ImGuiWindow* listWindow = ImGui::GetCurrentWindow(); // this is the listbox child window
            float list_h = ImGui::GetWindowHeight();

            // If your visual rows differ from indices (e.g. grouped/expanded rows),
            // replace `_selected` with the visual-row index:
            int visualIndex = _selected; // <- change this if you use visibleRows

            float target_y = visualIndex * item_h - (list_h * 0.5f) + (item_h * 0.5f);
            if (target_y < 0.0f) target_y = 0.0f;

            ImGui::SetScrollY(target_y);   // set the child window scroll BEFORE the clipper
            _jump_to = false;    // done
        }*/

        auto& instructions = _naratte->getInstructions();
        for (int i = 4235257; i <instructions.size(); i++) {
            render_node(i, 0);
        }


        ImGui::EndListBox();
    }
}

bool AsmView::render_node(int& id, int depth, bool visible) {
    auto& instructions = _naratte->getInstructions();
    std::string name = _naratte->getInstructionName(instructions[id].op);
    char label[128] = {};
    get_label(label, 128, id);

    if (instructions[id].isRet()) {
        if (visible) {
            if (instructions[id].isRet())
                ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(200, 255, 200, 255));
            //ImGui::Selectable(label);
            ImGui::TreeNodeEx(label, ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
            if (instructions[id].isRet())
                ImGui::PopStyleColor();
        }
        return true;
    }

    int end = id;
    for (int patternLen = 1; patternLen <= 10; patternLen++) {
        end = detect_pattern(id, patternLen);
        if (end != id) {
            if (visible)
                print_pattern(id, end, patternLen);
            if (id > 4335064)
                printf("pattern: %d - %d : %d\n", id, end, patternLen);
            break;
        }
    }
    if (end == id) {
        if (visible) {
            if (instructions[id].isRet())
                ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(200, 255, 200, 255));
            //ImGui::Selectable(label);
            ImGui::TreeNodeEx(label, ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
            if (instructions[id].isRet())
                ImGui::PopStyleColor();
        }
    }
    else
        id = end;

    if (instructions[id].isCall() || instructions[id].isJP()) {
#if 0
        bool vis = false;
        if (visible)
            vis = ImGui::TreeNode(std::string(label + std::to_string(id)).c_str());
        for (id += 1; id < instructions.size(); id++) {
            if (render_node(id, vis && visible))
                break;
        }
        if (vis)
            ImGui::TreePop();
#else
        if (ImGui::TreeNode(std::string(std::to_string(depth) + std::string(" ") + label + std::to_string(id)).c_str())) {
            for (id += 1; id < instructions.size(); id++) {
                if (render_node(id, depth + 1, true))
                    break;
            }
            ImGui::TreePop();
        }
        else {
            id = get_stack_return(id, depth);
        }
#endif

    }

    return false;
}

int AsmView::get_stack_return(const int id, const int depth) {
    auto callStack = _naratte->getCallStack();
    int stackSize = 0;
    for (const auto [stackId, type] : callStack){
        if (stackId < 4235257)
            continue;

        if (type == Naratte::CALL) {
            stackSize++;
            auto instruction = _naratte->getInstructions()[stackId];
            uint16_t addr = (instruction.op[2] << 8) | instruction.op[1];
            if (instruction.op[0] == 0xE9)
                addr = instruction.cpu.HL;
            //printf("CALL %d, %d %s\n", stackSize, stackId, _naratte->getCallLabel(addr).c_str());
        }
        else if (type == Naratte::RET) {
            stackSize--;
            //printf("RET %d, %d\n", stackSize, stackId);

            if (stackId > id && stackSize == depth)
                return stackId;
        }
    }

    return _naratte->getInstructions().size() - 2;
}

int AsmView::detect_pattern(const int pos, const int patternLen) const {
    auto& instructions = _naratte->getInstructions();
    if(pos + patternLen > instructions.size())
        return pos;

    std::vector<uint32_t> pattern(patternLen);
    for (int i = 0; i < patternLen; i++)
        pattern[i] = *reinterpret_cast<uint32_t*>(instructions[pos + i].op) & 0xFFFFFF;

    int end = pos;
    for (int i = 0; (pos + i) < instructions.size(); i++) {
        if (((*reinterpret_cast<uint32_t*>(instructions[pos + i].op)) & 0xFFFFFF) != pattern[i % patternLen]) {
            break;
        }

        if (i % patternLen == patternLen - 1)
            end = pos + i;
    }

    if ((end - pos) / patternLen < 3)
        return pos;

    return end;
}

void AsmView::print_pattern(const int pos, const int end, const int patternLen) const {
    ImGui::Separator();
    for (int i = 0; i < patternLen; i++) {
        char label[128] = {};
        get_label(label, 128, pos + i);
        ImGui::TreeNodeEx(label, ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
    }

    std::stringstream ss;
    ss << "Repeated ";
    ss << ((end - pos) / patternLen) - 1;
    ss << " times###";
    ss << pos;

    if (ImGui::TreeNode(ss.str().c_str())) {
        for (int i = pos + patternLen; i <= end; i++) {
            char label[128] = {};
            get_label(label, 128, i);
            ImGui::TreeNodeEx(label, ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
        }
        ImGui::TreePop();
    }
    ImGui::Separator();
}

void AsmView::jump_filter(bool next) {
    auto& instructions = _naratte->getInstructions();
    int selection = -1;
    int start = 0;
    if (_selected != -1) {
        start = next ? _selected + 1 : _selected - 1;
    }
    if (_jump_filter.empty()) {
        for (int i = start; next ? (i < instructions.size()) : (i >= 0); next ? i++ : i--) {
            std::string name = _naratte->getInstructionName(instructions[i].op);
            const bool found = name.find("CALL") != std::string::npos ||
                               name.find("JP")   != std::string::npos ||
                               name.find("RET")  != std::string::npos;
            if (found) {
                printf("%d\n", i);
                selection = i;
                break;
            }
        }
    }
    else {
        try {
            selection = std::stoi(_jump_filter);
        }
        catch (...) {
            printf("test\n");
            for (int i = start; next ? (i < instructions.size()) : (i >= 0); next ? i++ : i--) {
                std::string name = _naratte->getInstructionName(instructions[i].op);
                if (name.find(_jump_filter) != std::string::npos) {
                    selection = i;
                    break;
                }
            }
        }
    }


    if (selection != -1 && selection < instructions.size()) {
        _selected = selection;
        _jump_to = true;
    }
}
