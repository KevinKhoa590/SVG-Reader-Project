#pragma once
#include "SVGElement.h"
#include <string>

namespace Gdiplus {
    class Graphics;
    class Color;
}

class SVGText : public SVGElement {
private:
    int x, y;
    int fontSize;
    const Gdiplus::Color* fill;
    std::string text;

public:
    SVGText(int x, int y, int fontSize, const Gdiplus::Color* fill, const std::string& text);
    void draw(Gdiplus::Graphics& graphics) override;
};
