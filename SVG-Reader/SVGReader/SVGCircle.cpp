#include "stdafx.h"
#include "SVGCircle.h"
#include <windows.h>
#include <objidl.h>
#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")
using namespace Gdiplus;

SVGCircle::SVGCircle(int cx_, int cy_, int r_, const Color* fill, const Color* stroke, int strokeW)
    : cx(cx_), cy(cy_), r(r_), fillColor(fill), strokeColor(stroke), strokeWidth(strokeW) {}

void SVGCircle::draw(Graphics& g) {
    SolidBrush brush(*fillColor);
    g.FillEllipse(&brush, cx - r, cy - r, 2 * r, 2 * r);

    Pen pen(*strokeColor, strokeWidth);
    g.DrawEllipse(&pen, cx - r, cy - r, 2 * r, 2 * r);
}
