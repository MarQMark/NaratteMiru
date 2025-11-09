#ifndef NARATTEMIRU_CARTRIDGEINFOVIEW_H
#define NARATTEMIRU_CARTRIDGEINFOVIEW_H

#include <unordered_map>

#include "Naratte.h"
#include "Viewable.h"

class CartridgeInfoView : public Viewable{
public:
    explicit CartridgeInfoView(Naratte* naratte);

    void render() override;

private:
    Naratte* _naratte{};

    uint8_t read_mem(uint16_t addr) const;

    static const char* cartridge_type(uint8_t type);
    static const char* rom_size(uint8_t type);
    static const char* ram_size(uint8_t type);
    static const char* old_licensee_code(uint8_t type);
    static std::unordered_map<std::string, const char*> _new_licensee;
};


#endif //NARATTEMIRU_CARTRIDGEINFOVIEW_H