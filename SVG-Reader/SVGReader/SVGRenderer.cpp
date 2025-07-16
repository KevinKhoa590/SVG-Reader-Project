#include "stdafx.h"
#include "SVGRenderer.h"
#include <windows.h>
#include <objidl.h>
#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")

using namespace Gdiplus;

// Renders all elements contained in the SVGParseResult onto the provided GDI+ Graphics context
void SVGRenderer::render(const SVGParseResult& result, Gdiplus::Graphics& graphics) {
    // Iterate through each SVG element
    for (const auto& element : result.elements) {
        // Check that the element is valid before drawing
        if (element)
            element->draw(graphics);  // Delegate actual rendering to the element's draw() method
    }
}
