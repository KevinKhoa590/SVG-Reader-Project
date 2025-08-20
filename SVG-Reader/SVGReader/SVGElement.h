#pragma once

#include "transform.h"
#include "SVGGradient.h"
#include <string>
#include <memory>

namespace Gdiplus {
    class Graphics;
    class Color;
    class Brush;
    class RectF;
    class ColorBlend;
}

class SVGElement {
protected:
    Transform transform;
    std::wstring stroke = L"black";
    std::wstring fill = L"none";
    std::wstring strokeWidth = L"1";
    std::wstring strokeOpacity = L"1";
    std::wstring fillOpacity = L"1";

    SVGGradient* fillGradient = nullptr;
    SVGGradient* strokeGradient = nullptr;

public:
    virtual void draw(Gdiplus::Graphics& g) = 0;
    virtual ~SVGElement() {}

    // Transform
    void setTransform(const std::wstring& transformStr) {
        transform.applyTransform(transformStr);
    }

    const Transform& getTransform() const {
        return transform;
    }

    // Stroke
    void setStroke(const std::wstring& value) { stroke = value; }
    void setStrokeWidth(const std::wstring& value) { strokeWidth = value; }
    void setStrokeOpacity(const std::wstring& value) { strokeOpacity = value; }

    // Fill
    void setFill(const std::wstring& value) { fill = value; }
    void setFillOpacity(const std::wstring& value) { fillOpacity = value; }

    // Gradient setters/getters
    void setFillGradient(SVGGradient* g) { fillGradient = g; }
    void setStrokeGradient(SVGGradient* g) { strokeGradient = g; }
    SVGGradient* getFillGradient() const { return fillGradient; }
    SVGGradient* getStrokeGradient() const { return strokeGradient; }

    // Accessors (optional)
    std::wstring getStroke() const { return stroke; }
    std::wstring getStrokeWidth() const { return strokeWidth; }
    std::wstring getStrokeOpacity() const { return strokeOpacity; }
    std::wstring getFill() const { return fill; }
    std::wstring getFillOpacity() const { return fillOpacity; }

    void elementRender(Gdiplus::Graphics& g);
    std::unique_ptr<Gdiplus::Brush> createFillBrush(Gdiplus::Graphics& g, const Gdiplus::RectF& bounds) const;
    std::unique_ptr<Gdiplus::Brush> createStrokeBrush(Gdiplus::Graphics& g, const Gdiplus::RectF& bounds) const;
};
