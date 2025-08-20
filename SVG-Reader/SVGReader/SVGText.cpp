#include "stdafx.h"
#include "SVGText.h"
#include <windows.h>
#include <objidl.h>
#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")
using namespace Gdiplus;

SVGText::SVGText(int x, int y, int fontSize, const Color* fill, const std::string& text)
    : x(x), y(y), fontSize(fontSize), fillColor(fill), text(text) {}

void SVGText::draw(Gdiplus::Graphics& graphics) {
    RectF boundsF(static_cast<REAL>(x), static_cast<REAL>(y - fontSize), static_cast<REAL>(fontSize * text.size()), static_cast<REAL>(fontSize));

    auto brushPtr = createFillBrush(graphics, boundsF);
    FontFamily fontFamily(L"Arial");
    Font font(&fontFamily, static_cast<REAL>(fontSize), FontStyleRegular, UnitPixel);
    std::wstring wtext(text.begin(), text.end());
    PointF pt(static_cast<REAL>(x), static_cast<REAL>(y));

    if (brushPtr) {
        graphics.DrawString(wtext.c_str(), -1, &font, pt, brushPtr.get());
    }
    else if (fillColor) {
        SolidBrush brush(*fillColor);
        graphics.DrawString(wtext.c_str(), -1, &font, pt, &brush);
    }
}


