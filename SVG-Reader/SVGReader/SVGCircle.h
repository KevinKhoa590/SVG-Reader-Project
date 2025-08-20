
#pragma once
#include "SVGElement.h"

namespace Gdiplus {
    class Graphics;
    class Color;
}

class SVGCircle : public SVGElement {
    int cx, cy, r;
    const Gdiplus::Color* fillColor;
    const Gdiplus::Color* strokeColor;
    int strokeWidth;

public:
    SVGCircle(int cx, int cy, int r, const Gdiplus::Color* fill, const Gdiplus::Color* stroke, int strokeWidth);
    void draw(Gdiplus::Graphics& g) override;
};
