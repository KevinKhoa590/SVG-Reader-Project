#include "stdafx.h"
#include "SVGText.h"
#include <windows.h>
#include <objidl.h>
#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")
using namespace Gdiplus;

SVGText::SVGText(int x, int y, int fontSize, const Color* fill, const std::string& text)
    : x(x), y(y), fontSize(fontSize), fill(fill), text(text) {}

void SVGText::draw(Graphics& graphics) {
    FontFamily fontFamily(L"Arial");
    Font font(&fontFamily, static_cast<REAL>(fontSize), FontStyleRegular, UnitPixel);
    SolidBrush brush(*fill);

    std::wstring wtext(text.begin(), text.end());
    graphics.DrawString(wtext.c_str(), -1, &font, PointF(static_cast<REAL>(x), static_cast<REAL>(y)), &brush);
}
