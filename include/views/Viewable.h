#ifndef NARATTEMIRU_VIEWABLE_H
#define NARATTEMIRU_VIEWABLE_H

class Viewable {
public:
    virtual ~Viewable() = default;

    virtual void render() = 0;
    void* view{};

    virtual void setVisible(const bool visible) {
        _visible = visible;
    };
    bool isVisible() const{
        return _visible;
    }

protected:
    bool _visible = true;
};

#endif //NARATTEMIRU_VIEWABLE_H
