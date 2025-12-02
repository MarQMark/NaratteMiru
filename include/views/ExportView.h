#ifndef NARATTEMIRU_EXPORTVIEW_H
#define NARATTEMIRU_EXPORTVIEW_H

#include "Naratte.h"
#include "Viewable.h"

class ExportView : public Viewable{
public:
    explicit ExportView(Naratte* naratte);

    void render() override;

private:
    Naratte* _naratte{};

    std::string _path{};
    std::string _name = "save.nm";
};


#endif //NARATTEMIRU_EXPORTVIEW_H