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
void SVGRect::draw(Graphics& g) {
    // Create a brush with the fill color and draw the filled rectangle
    SolidBrush brush(*fillColor);
    g.FillRectangle(&brush, x, y, width, height);

    // Create a pen with the stroke color and width, and draw the border
    Pen pen(*strokeColor, strokeWidth);
    g.DrawRectangle(&pen, x, y, width, height);
}
