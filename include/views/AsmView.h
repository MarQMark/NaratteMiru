#ifndef NARATTEMIRU_ASMVIEW_H
#define NARATTEMIRU_ASMVIEW_H

#include <cstdint>
#include <string>
#include <vector>

#include "Viewable.h"
#include "Naratte.h"

class AsmView : public Viewable {
public:
    explicit AsmView(Naratte* naratte);

    void render() override;
private:
    Naratte* _naratte{};
    long int _selected = -1;
    void get_label(char* label, size_t len, int id) const;

    bool _format = false;
    void render_raw();
    void render_format();
    bool render_node(int& id, int depth, bool visible = true);
    int get_stack_return(int id, int depth);
    int detect_pattern(int pos, int patternLen) const;
    void print_pattern(int pos, int end, int patternLen) const;

    std::string _jump_filter;

    bool _jump_to = false;
    void jump_filter(bool next);
};


#endif //NARATTEMIRU_ASMVIEW_H