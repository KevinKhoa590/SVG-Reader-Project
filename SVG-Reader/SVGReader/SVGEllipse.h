#pragma once
#include "SVGElement.h"

namespace Gdiplus {
    class Graphics;
    class Color;
}

// Represents an SVG <ellipse> element
class SVGEllipse : public SVGElement {
    int cx, cy;                         // Center coordinates of the ellipse
    int rx, ry;                         // Radii along the x and y axes
    const Gdiplus::Color* fillColor;    // Fill color (nullable pointer)
    const Gdiplus::Color* strokeColor;  // Stroke color (nullable pointer)
    int strokeWidth;                    // Stroke thickness

public:
    // Constructs an ellipse with center (cx, cy), radii (rx, ry), colors, and stroke width
    SVGEllipse(int cx, int cy, int rx, int ry,
        const Gdiplus::Color* fill,
        const Gdiplus::Color* stroke,
        int strokeWidth);

    // Draws the ellipse on a given GDI+ Graphics context
    void draw(Gdiplus::Graphics& g) override;
};
