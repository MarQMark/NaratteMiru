#include "views/AsmView.h"

#include <algorithm>
#include <ranges>
#include <sstream>

#include "Config.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "misc/cpp/imgui_stdlib.h"

const uint8_t Instruction::type[256] = {
    7,2,1,4, 3,3,1,5, 4,4,1,4, 3,3,1,5,
    7,2,1,4, 3,3,1,5, 6,4,1,4, 3,3,1,5,
    6,2,1,4, 3,3,1,3, 6,4,1,4, 3,3,1,3,
    6,2,1,4, 3,3,1,3, 6,4,1,4, 3,3,1,3,

    1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1,
    1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1,
    1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1,
    1,1,1,1, 1,1,6,1, 1,1,1,1, 1,1,1,1,

    3,3,3,3, 3,3,3,3, 3,3,3,3, 3,3,3,3,
    3,3,3,3, 3,3,3,3, 3,3,3,3, 3,3,3,3,
    3,3,3,3, 3,3,3,3, 3,3,3,3, 3,3,3,3,
    3,3,3,3, 3,3,3,3, 3,3,3,3, 3,3,3,3,

    6,2,6,6, 6,2,3,6, 6,6,6,5, 6,6,3,6,
    6,2,6,0, 6,2,3,6, 6,6,6,0, 6,0,3,6,
    1,2,1,0, 0,2,3,6, 4,6,1,0, 0,0,3,6,
    1,2,1,7, 0,2,3,6, 2,2,1,7, 0,0,3,6
};

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
    if (ctrl && ImGui::IsKeyPressed(ImGuiKey_UpArrow))
        jump_filter(false);
    else if (ImGui::IsKeyPressed(ImGuiKey_UpArrow)) {
        _selected = (_selected - 1) < 0 ? ((int)_naratte->getInstructions().size() - 1) : _selected - 1;
        _jump_to = true;
    }
    if (ctrl && ImGui::IsKeyPressed(ImGuiKey_DownArrow))
        jump_filter(true);
    else if (ImGui::IsKeyPressed(ImGuiKey_DownArrow)) {
        _selected = (_selected + 1) % (int)_naratte->getInstructions().size();
        _jump_to = true;
    }
    if (ctrl && ImGui::IsKeyPressed(ImGuiKey_R))
        _naratte->reload();

    ImGui::Separator();

    if (!_format)
        render_raw();
    else
        render_format();

    ImGui::Separator();
    _selected == -1 ? ImGui::Text("Selected: None") : ImGui::Text("Selected: %lu", _selected);
    ImGui::SameLine(); ImGui::Text("(%lu)", _naratte->getInstructions().size());
    if(_selected != -1) {
        ImGui::SameLine();
        char* name = _naratte->getInstructionName(_naratte->getInstructions()[_selected].op);
        ImGui::Text(name);
    }
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

    ImGui::Begin("Call Stack");
    if (ImGui::BeginListBox("##instr_lis3t", ImVec2(-FLT_MIN, -FLT_MIN))) {
        const auto& completeCS = _naratte->getCallStack();
        std::vector<std::pair<int, int>> callStack;
        int i = 0;
        // find last "function" call in stack to selected
        for (; i < completeCS.size(); i++) {
            if (_selected < completeCS[i].first) {
                i--;
                break;
            }
        }
        if (i == completeCS.size())
            i--;

        int maxDepth = 0; // In reverse
        int currentDepth = 0;
        for (; i >= 0; i--) {
            const auto& [idx, type] = completeCS[i];

            if (type == Naratte::CALL)
                currentDepth++;
            else if (type == Naratte::RET)
                currentDepth--;

            if (currentDepth > maxDepth) {
                maxDepth = currentDepth;

                if (type == Naratte::CALL)
                    callStack.emplace_back(std::pair{idx, type});
            }
        }
        std::ranges::reverse(callStack);

        int callDepth = 0;
        for (const auto &idx: callStack | std::views::keys) {
            if (idx < _start)
                continue;

            if (_selected < idx)
                break;

            const auto&[op, cpu] = _naratte->getInstructions()[idx];
            uint16_t addr = (op[2] << 8) | op[1];
            if (op[0] == 0xE9)
                addr = cpu.HL;

            char label[128]{};
            sprintf(label, "%*s%s (%d)##CallStack", callDepth, "", _naratte->getCallLabel(addr).c_str(), idx);
            ImGui::Selectable(label);
            add_context_menu(idx);

            callDepth++;
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
    auto height = ImGui::GetContentRegionAvail().y;
    if (ImGui::BeginListBox("##instr_list", ImVec2(-FLT_MIN, height - 21))) {
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
                render_selectable(i);
            }
        }

        ImGui::EndListBox();
    }
}

void AsmView::render_format() {
    auto height = ImGui::GetContentRegionAvail().y;
    if (ImGui::BeginListBox("##instr_list", ImVec2(-FLT_MIN, height - 21))) {
        auto& instructions = _naratte->getInstructions();
        for (int i = _start; i < instructions.size(); i++) {
            render_node(i, 0);
        }

        ImGui::EndListBox();
    }
}

bool AsmView::render_node(int& id, int depth) {
    auto& instructions = _naratte->getInstructions();
    char label[128] = {};
    get_label(label, 128, id);

    // RET detected -> go up in hierarchy
    if (instructions[id].isRet()) {
        render_tree_node(id, true);
        return true;
    }

    // Detect Pattern and group it
    int end = id;
    for (int patternLen = 1; patternLen <= 10; patternLen++) {
        end = detect_pattern(id, patternLen);
        if (end != id) {
            print_pattern(id, end, patternLen);
            break;
        }
    }

    // No pattern detected
    if (end == id) {

        if (instructions[id].isCall() ||
           (instructions[id].isJP() && Config::get()->isJPasCall(id, instructions.size() - 1))) {
            if (_selected > id && _selected <= get_ret_from_call(id))
                ImGui::SetNextItemOpen(true, ImGuiCond_Always);

            if (render_tree_node(id, false)) {
                for (id += 1; id < instructions.size(); id++) {
                    if (render_node(id, depth + 1))
                        break;
                }
                ImGui::TreePop();
            }
            else {
                // Don't render no open instructions of a "function"
                id = get_stack_return(id, depth);
            }
        }
        else {
            // Just normal node (no pattern, no RET, no CALL/JP)
            render_tree_node(id, true);
        }
    }
    // Skip grouped pattern
    else
        id = end;


    return false;
}

int AsmView::get_stack_return(const int id, const int depth) const {
    auto callStack = _naratte->getCallStack();
    int stackSize = 0;
    for (const auto [stackId, type] : callStack){
        if (stackId < _start)
            continue;

        if (type == Naratte::CALL) {
            stackSize++;
        }
        else if (type == Naratte::RET) {
            stackSize--;

            if (stackId > id && stackSize == depth)
                return stackId;
        }
    }

    return _naratte->getInstructions().size() - 1;
}

int AsmView::get_ret_from_call(const int id) const {
    int depth = -1;
    for (const auto& [idx, type] : _naratte->getCallStack()) {
        if (idx == id)
            depth = 0;

        if (depth != -1) {
            if (type == Naratte::CALL)
                depth++;
            else if (type == Naratte::RET)
                depth--;

            if (depth == 0)
                return idx;
        }
    }

    return _naratte->getInstructions().size() - 1;
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

void AsmView::print_pattern(const int pos, const int end, const int patternLen) {
    ImGui::Separator();
    for (int i = 0; i < patternLen; i++) {
        render_tree_node(pos + i, true);
    }

    std::stringstream ss;
    ss << "Repeated ";
    ss << ((end - pos) / patternLen);
    ss << " times###";
    ss << pos;

    ImGuiTreeNodeFlags flags = 0;
    if (_selected >= pos + patternLen && _selected <= end)
        flags |= ImGuiTreeNodeFlags_Selected;

    if (ImGui::TreeNodeEx(ss.str().c_str(), flags)) {
        for (int i = pos + patternLen; i <= end; i++) {
            render_tree_node(i, true);
        }
        ImGui::TreePop();
    }
    ImGui::Separator();
}

static constexpr ImVec4 palette[] = {
    ImVec4(0.60f, 0.60f, 0.60f, 1.00f), // grey
    ImVec4(0.26f, 0.59f, 0.98f, 1.00f), // blue
    ImVec4(0.30f, 0.85f, 0.50f, 1.00f), // green
    ImVec4(0.90f, 0.85f, 0.30f, 1.00f), // yellow
    ImVec4(1.00f, 0.45f, 0.70f, 1.00f), // pink
    ImVec4(0.30f, 0.90f, 0.90f, 1.00f), // cyan
    ImVec4(1.00f, 0.65f, 0.30f, 1.00f), // orange
    ImVec4(0.70f, 0.50f, 0.90f, 1.00f), // purple
};

bool AsmView::render_tree_node(const int id, const bool leaf) {
    const auto instruction = _naratte->getInstructions()[id];
    ImGui::PushStyleColor(ImGuiCol_Text, palette[instruction.getType()]);

    char label[128] = {};
    get_label(label, 128, id);

    if (id == _selected && _jump_to) {
        const auto windowPos = ImGui::GetCursorPosY();
        ImGui::SetScrollY(windowPos);
        _jump_to = false;
    }

    const bool open = ImGui::TreeNodeEx(label,
        (leaf ? ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen: 0 ) |
        (id == _selected ? ImGuiTreeNodeFlags_Selected : 0));

    if (ImGui::IsItemClicked()) {
        _selected = id;
    }

    add_context_menu(id);
    ImGui::PopStyleColor();
    return open;
}

void AsmView::render_selectable(const int id) {
    const auto instruction = _naratte->getInstructions()[id];
    ImGui::PushStyleColor(ImGuiCol_Text, palette[instruction.getType()]);

    const bool is_selected = (_selected == id);
    char label[128] = {};
    get_label(label, 128, id);

    if (ImGui::Selectable(label, is_selected))
        _selected = id;
    add_context_menu(id);
    if (is_selected)
        ImGui::SetItemDefaultFocus();

    ImGui::PopStyleColor();
}

void AsmView::add_context_menu(const int id) {
    if (ImGui::BeginPopupContextItem()) {
        if (ImGui::MenuItem("Copy")) {
            ImGui::SetClipboardText(std::to_string(id).c_str());
        }
        ImGui::EndPopup();
    }
}

void AsmView::jump_filter(bool next) {
    auto& instructions = _naratte->getInstructions();
    int selection = -1;
    int start = 0;
    if (_selected != -1) {
        start = next ? _selected + 1 : _selected - 1;
    }
    if (_jump_filter.empty()) {
        for (int i = start; (i < instructions.size()) && (i >= 0); next ? i++ : i--) {
            if (instructions[i].isRet() || instructions[i].isCall() || instructions[i].isJP()) {
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
            for (int i = start; (i < instructions.size()) && (i >= 0); next ? i++ : i--) {
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
