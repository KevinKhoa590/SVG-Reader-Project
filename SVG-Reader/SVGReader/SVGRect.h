#pragma once
#include "SVGElement.h"

namespace Gdiplus {
    class Graphics;
    class Color;
}

// Represents an SVG <rect> element
class SVGRect : public SVGElement {
    int x, y;                        // Top-left corner position
    int width, height;              // Dimensions of the rectangle
    const Gdiplus::Color* fillColor;    // Pointer to fill color
    const Gdiplus::Color* strokeColor;  // Pointer to stroke (border) color
    int strokeWidth;                // Width of the stroke (border)

public:
    // Constructor to initialize rectangle properties
    SVGRect(int x, int y, int width, int height,
        const Gdiplus::Color* fill,
        const Gdiplus::Color* stroke,
        int strokeWidth);

    // Overrides the draw function to render the rectangle
    void draw(Gdiplus::Graphics& g) override;
};
