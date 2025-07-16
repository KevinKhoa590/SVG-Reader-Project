#pragma once
#include "stdafx.h"
#include "SVGParser.h"

namespace Gdiplus {
    class Graphics;  // Forward declaration to avoid full GDI+ dependency in header
}

// SVGRenderer is responsible for drawing parsed SVG elements to a GDI+ Graphics context
class SVGRenderer {
public:
    // Render all SVG elements from a parsed result onto the specified graphics context
    void render(const SVGParseResult& result, Gdiplus::Graphics& graphics);
};
