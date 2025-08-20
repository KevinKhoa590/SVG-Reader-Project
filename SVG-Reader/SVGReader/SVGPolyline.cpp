#include "stdafx.h"
#include "SVGPolyline.h"
#include <windows.h>
#include <objidl.h>
#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")
using namespace Gdiplus;

SVGPolyline::SVGPolyline(const std::vector<Point>& pts, const Color* fill, const Color* stroke, int strokeW)
    : points(pts), fillColor(fill), strokeColor(stroke), strokeWidth(strokeW) {}

void SVGPolyline::draw(Graphics& g) {
    if (points.empty()) return;

    REAL minX = points[0].X, minY = points[0].Y, maxX = points[0].X, maxY = points[0].Y;
    for (size_t i = 1; i < points.size(); ++i) {
        if (points[i].X < minX) minX = points[i].X;
        if (points[i].Y < minY) minY = points[i].Y;
        if (points[i].X > maxX) maxX = points[i].X;
        if (points[i].Y > maxY) maxY = points[i].Y;
    }
    RectF boundsF(minX, minY, maxX - minX, maxY - minY);

    auto brushPtr = createFillBrush(g, boundsF);
    if (brushPtr) {
        g.FillPolygon(brushPtr.get(), points.data(), (INT)points.size());
    }
    else if (fillColor) {
        SolidBrush brush(*fillColor);
        g.FillPolygon(&brush, points.data(), (INT)points.size());
    }

    if ((strokeColor != nullptr || getStrokeGradient() != nullptr) && strokeWidth > 0) {
        auto strokeBrush = createStrokeBrush(g, boundsF);
        if (strokeBrush) {
            Pen pen(strokeBrush.get(), static_cast<REAL>(strokeWidth));
            g.DrawLines(&pen, points.data(), (INT)points.size());
        }
        else if (strokeColor) {
            Pen pen(*strokeColor, static_cast<REAL>(strokeWidth));
            g.DrawLines(&pen, points.data(), (INT)points.size());
        }
    }
}

