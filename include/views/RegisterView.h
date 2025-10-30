#ifndef NARATTEMIRU_REGISTERVIEW_H
#define NARATTEMIRU_REGISTERVIEW_H

#include "Naratte.h"
#include "Viewable.h"

class RegisterView : public Viewable{
public:
    explicit RegisterView(Naratte* naratte);

    void render() override;
private:
    Naratte* _naratte{};
};


#endif //NARATTEMIRU_REGISTERVIEW_H