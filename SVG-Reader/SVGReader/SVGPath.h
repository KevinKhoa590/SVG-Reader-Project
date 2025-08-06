#pragma once

#include "SVGElement.h"
#include "helper.h"
#include <vector>
#include <string>

namespace Gdiplus {
    class Color;
}

class SVGPath : public SVGElement { 
private:
    std::wstring d;

    const Gdiplus::Color *strokeColor;
    const Gdiplus::Color *fillColor;
    bool hasStroke = false;
    bool hasFill = false;

    float strokeWidth = 1.0f;
    float strokeOpacity = 1.0f;
    float fillOpacity = 1.0f;

public:
    SVGPath();
    ~SVGPath() override;

    void setAttribute(xml_attribute<>* attr);
    void draw(Gdiplus::Graphics& g) override;
};
