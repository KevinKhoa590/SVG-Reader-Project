#include "stdafx.h"
#include "SVGPolygon.h"
#include <windows.h>
#include <objidl.h>
#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")
using namespace Gdiplus;

SVGPolygon::SVGPolygon(const std::vector<Point>& pts, const Color* fill, const Color* stroke, int strokeW)
    : points(pts), fillColor(fill), strokeColor(stroke), strokeWidth(strokeW) {}

void SVGPolygon::draw(Graphics& g) {
    if (points.empty()) return;

    SolidBrush brush(*fillColor);
    g.FillPolygon(&brush, points.data(), points.size());

    Pen pen(*strokeColor, strokeWidth);
    g.DrawPolygon(&pen, points.data(), points.size());
}
