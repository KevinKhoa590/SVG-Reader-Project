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
    REAL left = static_cast<REAL>(cx - r);
    REAL top = static_cast<REAL>(cy - r);
    REAL diameter = static_cast<REAL>(2 * r);
    RectF boundsF(left, top, diameter, diameter);

    auto brushPtr = createFillBrush(g, boundsF);
    if (brushPtr) {
        g.FillEllipse(brushPtr.get(), left, top, diameter, diameter);
    }
    else if (fillColor) {
        SolidBrush brush(*fillColor);
        g.FillEllipse(&brush, left, top, diameter, diameter);
    }

    if ((strokeColor != nullptr || getStrokeGradient() != nullptr) && strokeWidth > 0) {
        auto strokeBrush = createStrokeBrush(g, boundsF);
        if (strokeBrush) {
            Pen pen(strokeBrush.get(), static_cast<REAL>(strokeWidth));
            g.DrawEllipse(&pen, left, top, diameter, diameter);
        }
        else if (strokeColor) {
            Pen pen(*strokeColor, static_cast<REAL>(strokeWidth));
            g.DrawEllipse(&pen, left, top, diameter, diameter);
        }
    }
}

