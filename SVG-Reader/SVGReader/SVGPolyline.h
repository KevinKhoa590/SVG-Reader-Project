#pragma once
#include "SVGElement.h"
#include <vector>

namespace Gdiplus {
    class Graphics;
    class Color;
    class Point;
}

class SVGPolyline : public SVGElement {
    std::vector<Gdiplus::Point> points;
    const Gdiplus::Color* fillColor;
    const Gdiplus::Color* strokeColor;
    int strokeWidth;

public:
    SVGPolyline(const std::vector<Gdiplus::Point>& pts, const Gdiplus::Color* fill, const Gdiplus::Color* stroke, int strokeWidth);
    void draw(Gdiplus::Graphics& g) override;
};
