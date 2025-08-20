#include "stdafx.h"
#include "SVGLine.h"
#include <windows.h>
#include <objidl.h>
#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")
using namespace Gdiplus;

SVGLine::SVGLine(int x1_, int y1_, int x2_, int y2_, const Color* color_, int width_)
    : x1(x1_), y1(y1_), x2(x2_), y2(y2_), strokeColor(color_), strokeWidth(width_) {}

void SVGLine::draw(Gdiplus::Graphics& g) {
    RectF boundsF((std::min)(x1, x2), (std::min)(y1, y2), (std::abs)(x2 - x1), (std::abs)(y2 - y1));

    if ((strokeColor != nullptr || getStrokeGradient() != nullptr) && strokeWidth > 0) {
        auto strokeBrush = createStrokeBrush(g, boundsF);
        if (strokeBrush) {
            Pen pen(strokeBrush.get(), static_cast<REAL>(strokeWidth));
            g.DrawLine(&pen, x1, y1, x2, y2);
        }
        else if (strokeColor) {
            Pen pen(*strokeColor, static_cast<REAL>(strokeWidth));
            g.DrawLine(&pen, x1, y1, x2, y2);
        }
    }
}

