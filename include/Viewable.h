#ifndef NARATTEMIRU_VIEWABLE_H
#define NARATTEMIRU_VIEWABLE_H

class Viewable {
public:
    virtual ~Viewable() = default;

    virtual void render() = 0;
};

#endif //NARATTEMIRU_VIEWABLE_H
