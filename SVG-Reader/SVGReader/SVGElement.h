#pragma once

namespace Gdiplus {
    class Graphics;
}

class SVGElement {
public:
    virtual void draw(Gdiplus::Graphics& g) = 0;
    virtual ~SVGElement() {}
};
