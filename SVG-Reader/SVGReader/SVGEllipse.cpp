#include "stdafx.h"
#include "SVGEllipse.h"
#include <windows.h>
#include <objidl.h>
#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")

using namespace Gdiplus;

SVGEllipse::SVGEllipse(int cx_, int cy_, int rx_, int ry_,
    const Color* fill, const Color* stroke, int strokeW)
    : cx(cx_), cy(cy_), rx(rx_), ry(ry_),
    fillColor(fill), strokeColor(stroke), strokeWidth(strokeW) {}

void SVGEllipse::draw(Gdiplus::Graphics& g) {
    REAL left = static_cast<REAL>(cx - rx);
    REAL top = static_cast<REAL>(cy - ry);
    REAL w = static_cast<REAL>(2 * rx);
    REAL h = static_cast<REAL>(2 * ry);
    RectF boundsF(left, top, w, h);

    auto brushPtr = createFillBrush(g, boundsF);
    if (brushPtr) {
        g.FillEllipse(brushPtr.get(), left, top, w, h);
    }
    else if (fillColor) {
        SolidBrush brush(*fillColor);
        g.FillEllipse(&brush, left, top, w, h);
    }

    if ((strokeColor != nullptr || getStrokeGradient() != nullptr) && strokeWidth > 0) {
        auto strokeBrush = createStrokeBrush(g, boundsF);
        if (strokeBrush) {
            Pen pen(strokeBrush.get(), static_cast<REAL>(strokeWidth));
            g.DrawEllipse(&pen, left, top, w, h);
        }
        else if (strokeColor) {
            Pen pen(*strokeColor, static_cast<REAL>(strokeWidth));
            g.DrawEllipse(&pen, left, top, w, h);
        }
    }
}


