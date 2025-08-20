#include "stdafx.h"
#include "SVGRect.h"
#include <windows.h>
#include <objidl.h>
#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")
using namespace Gdiplus;

// Constructor: initializes rectangle attributes
SVGRect::SVGRect(int x_, int y_, int w_, int h_,
    const Color* fill, const Color* stroke, int strokeW)
    : x(x_), y(y_), width(w_), height(h_),
    fillColor(fill), strokeColor(stroke), strokeWidth(strokeW) {}

// Draws the rectangle using GDI+ Graphics context
void SVGRect::draw(Gdiplus::Graphics& g) {
    RectF boundsF(static_cast<REAL>(x), static_cast<REAL>(y),
        static_cast<REAL>(width), static_cast<REAL>(height));

    // Fill
    {
        auto brushPtr = createFillBrush(g, boundsF);
        if (brushPtr) {
            g.FillRectangle(brushPtr.get(), boundsF);
        }
        else if (fillColor) {
            SolidBrush sb(*fillColor);
            g.FillRectangle(&sb, boundsF);
        }
    }

    // Stroke
    if ((strokeColor != nullptr || getStrokeGradient() != nullptr) && strokeWidth > 0) {
        auto strokeBrush = createStrokeBrush(g, boundsF);
        if (strokeBrush) {
            Pen pen(strokeBrush.get(), static_cast<REAL>(strokeWidth));
            g.DrawRectangle(&pen, boundsF);
        }
        else if (strokeColor) {
            Pen pen(*strokeColor, static_cast<REAL>(strokeWidth));
            g.DrawRectangle(&pen, boundsF);
        }
    }
}


