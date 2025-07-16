#pragma once
#include "SVGElement.h"

namespace Gdiplus {
    class Graphics;
    class Color;
}

class SVGLine : public SVGElement {
    int x1, y1, x2, y2;
    const Gdiplus::Color* strokeColor;
    int strokeWidth;

public:
    SVGLine(int x1, int y1, int x2, int y2, const Gdiplus::Color* color, int width);
    void draw(Gdiplus::Graphics& g) override;
};
