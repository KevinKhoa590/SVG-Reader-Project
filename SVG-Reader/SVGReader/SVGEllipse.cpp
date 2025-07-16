#include "stdafx.h"
#include "SVGEllipse.h"
#include <windows.h>
#include <objidl.h>
#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")

using namespace Gdiplus;

// Constructor to initialize the ellipse properties
SVGEllipse::SVGEllipse(int cx_, int cy_, int rx_, int ry_,
    const Color* fill, const Color* stroke, int strokeW)
    : cx(cx_), cy(cy_), rx(rx_), ry(ry_),
    fillColor(fill), strokeColor(stroke), strokeWidth(strokeW) {}

// Draws the ellipse using GDI+
// The ellipse is drawn by converting center/radius to bounding box
void SVGEllipse::draw(Graphics& g) {
    // Create brush with fill color and draw the filled ellipse
    SolidBrush brush(*fillColor);
    g.FillEllipse(&brush, cx - rx, cy - ry, 2 * rx, 2 * ry);

    // Create pen with stroke color and draw the ellipse border
    Pen pen(*strokeColor, strokeWidth);
    g.DrawEllipse(&pen, cx - rx, cy - ry, 2 * rx, 2 * ry);
}
