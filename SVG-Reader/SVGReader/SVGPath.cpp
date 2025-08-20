#include "stdafx.h"
#include "SVGPath.h"
#include "helper.h"
#include <windows.h>
#include <objidl.h>
#include <gdiplus.h>
#include <sstream>
#pragma comment(lib, "gdiplus.lib")
using namespace Gdiplus;

SVGPath::SVGPath()
    : strokeColor(nullptr), fillColor(nullptr),
    strokeWidth(1.0f), strokeOpacity(1.0f), fillOpacity(1.0f),
    hasStroke(false), hasFill(false) {}


SVGPath::~SVGPath() {
    delete strokeColor;
    delete fillColor;
}

void SVGPath::setAttribute(xml_attribute<>* attr) {
    std::wstring name = std::wstring(attr->name(), attr->name() + strlen(attr->name()));
    std::wstring value = std::wstring(attr->value(), attr->value() + strlen(attr->value()));

    if (name == L"d") {
        d = value;
    }
    else if (name == L"stroke") {
        delete strokeColor;
        strokeColor = new Color(parseColor(attr));
        hasStroke = true;
    }
    else if (name == L"fill") {
        delete fillColor;
        fillColor = new Color(parseColor(attr));
        hasFill = true;
    }
    else if (name == L"stroke-width") {
        strokeWidth = std::stof(value);
    }
    else if (name == L"stroke-opacity") {
        strokeOpacity = parseOpacity(attr);
    }
    else if (name == L"fill-opacity") {
        fillOpacity = parseOpacity(attr);
    }
    else if (name == L"fill") {
        if (value == L"none") {
            delete fillColor;
            fillColor = nullptr;
            hasFill = false;
        }
        else {
            delete fillColor;
            fillColor = new Gdiplus::Color(parseColor(attr));
            hasFill = true;
        }
    }
}

void SVGPath::draw(Graphics& g) {
    GraphicsPath path;
    std::wstringstream ss(d);
    wchar_t cmd;
    PointF curr{ 0, 0 }, start{ 0, 0 };

    while (ss >> cmd) {
        if (cmd == L'M' || cmd == L'm') {
            float x, y;
            ss >> x; if (ss.peek() == L',') ss.ignore();
            ss >> y;
            if (cmd == L'm') {
                curr.X += x;
                curr.Y += y;
            }
            else {
                curr = { x, y };
            }
            start = curr;
        }
        else if (cmd == L'L' || cmd == L'l') {
            float x, y;
            ss >> x; if (ss.peek() == L',') ss.ignore();
            ss >> y;
            PointF dest = (cmd == L'l') ? PointF{ curr.X + x, curr.Y + y } : PointF{ x, y };
            path.AddLine(curr, dest);
            curr = dest;
        }
        else if (cmd == L'H' || cmd == L'h') {
            float x;
            ss >> x;
            float newX = (cmd == L'h') ? curr.X + x : x;
            PointF dest = { newX, curr.Y };
            path.AddLine(curr, dest);
            curr = dest;
        }
        else if (cmd == L'V' || cmd == L'v') {
            float y;
            ss >> y;
            float newY = (cmd == L'v') ? curr.Y + y : y;
            PointF dest = { curr.X, newY };
            path.AddLine(curr, dest);
            curr = dest;
        }
        else if (cmd == L'C' || cmd == L'c') {
            float x1, y1, x2, y2, x, y;
            ss >> x1; if (ss.peek() == L',') ss.ignore();
            ss >> y1; if (ss.peek() == L',') ss.ignore();
            ss >> x2; if (ss.peek() == L',') ss.ignore();
            ss >> y2; if (ss.peek() == L',') ss.ignore();
            ss >> x;  if (ss.peek() == L',') ss.ignore();
            ss >> y;

            PointF p1 = (cmd == L'c') ? PointF{ curr.X + x1, curr.Y + y1 } : PointF{ x1, y1 };
            PointF p2 = (cmd == L'c') ? PointF{ curr.X + x2, curr.Y + y2 } : PointF{ x2, y2 };
            PointF dest = (cmd == L'c') ? PointF{ curr.X + x, curr.Y + y } : PointF{ x, y };

            path.AddBezier(curr, p1, p2, dest);
            curr = dest;
        }
        else if (cmd == L'Z' || cmd == L'z') {
            path.CloseFigure();
            curr = start;
        }
    }

    // Fill
    RectF bounds;
    path.GetBounds(&bounds);

    if (getFillGradient()) {
        auto brush = createFillBrush(g, bounds);
        if (brush) g.FillPath(brush.get(), &path);
    }
    else if (fillColor && fillOpacity > 0.0f) {
        SolidBrush brush(Color(
            static_cast<BYTE>(clamp01(fillOpacity) * 255.0f),
            fillColor->GetR(),
            fillColor->GetG(),
            fillColor->GetB()
        ));
        g.FillPath(&brush, &path);
    }

    // Stroke
    if (strokeColor && strokeWidth > 0 && strokeOpacity > 0.0f) {
        Pen pen(Color(
            static_cast<BYTE>(clamp01(strokeOpacity) * 255.0f),
            strokeColor->GetR(),
            strokeColor->GetG(),
            strokeColor->GetB()
        ), static_cast<REAL>(strokeWidth));
        g.DrawPath(&pen, &path);
    }
}
