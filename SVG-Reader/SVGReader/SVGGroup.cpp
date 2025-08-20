#include "stdafx.h"
#include "SVGGroup.h"
#include "helper.h"
#include <windows.h>
#include <objidl.h>
#include <gdiplus.h>

#pragma comment(lib, "gdiplus.lib")

using namespace Gdiplus;

SVGGroup::SVGGroup() {}

SVGGroup::~SVGGroup() {
    children.clear(); // smart pointers auto-clean
}

void SVGGroup::addElement(std::unique_ptr<SVGElement> child) {
    children.push_back(std::move(child));
}

void SVGGroup::setTransform(Matrix* matrix) {
    transform.reset(matrix); // take ownership safely
}

void SVGGroup::elementRender(Graphics& g) {
    GraphicsState state = g.Save(); // save graphics state

    if (transform) {
        // Apply group's transform
        g.MultiplyTransform(transform.get(), MatrixOrderPrepend);
    }

    // Draw group (this will call draw which iterates children)
    draw(g);

    g.Restore(state); // restore graphics state
}

void SVGGroup::draw(Graphics& g) {
    for (auto& child : children) {
        if (!child) continue;

        // Save original child presentation attributes
        std::wstring origStroke = child->getStroke();
        std::wstring origFill = child->getFill();
        std::wstring origStrokeWidth = child->getStrokeWidth();
        std::wstring origStrokeOpacity = child->getStrokeOpacity();
        std::wstring origFillOpacity = child->getFillOpacity();

        bool mutated = false;

        // Inherit fill if child has default "none" (or empty) and group provides a fill
        if ((origFill == L"none" || origFill.empty()) && (this->getFill() != L"none" && !this->getFill().empty())) {
            child->setFill(this->getFill());
            mutated = true;
        }

        // Inherit stroke if child has default "black" or empty and group provides a stroke
        if ((origStroke == L"black" || origStroke.empty()) && (this->getStroke() != L"black" && !this->getStroke().empty())) {
            child->setStroke(this->getStroke());
            mutated = true;
        }

        // Inherit stroke-width if child has default "1" or empty and group provides different value
        if ((origStrokeWidth == L"1" || origStrokeWidth.empty()) && (this->getStrokeWidth() != L"1" && !this->getStrokeWidth().empty())) {
            child->setStrokeWidth(this->getStrokeWidth());
            mutated = true;
        }

        // Inherit stroke-opacity if child has default "1" or empty and group provides different
        if ((origStrokeOpacity == L"1" || origStrokeOpacity.empty()) && (this->getStrokeOpacity() != L"1" && !this->getStrokeOpacity().empty())) {
            child->setStrokeOpacity(this->getStrokeOpacity());
            mutated = true;
        }

        // Inherit fill-opacity if child has default "1" or empty and group provides different
        if ((origFillOpacity == L"1" || origFillOpacity.empty()) && (this->getFillOpacity() != L"1" && !this->getFillOpacity().empty())) {
            child->setFillOpacity(this->getFillOpacity());
            mutated = true;
        }

        // Render the child with any temporary inherited attributes
        child->elementRender(g);

        // Restore child's original attributes to avoid permanent mutation
        if (mutated) {
            child->setStroke(origStroke);
            child->setFill(origFill);
            child->setStrokeWidth(origStrokeWidth);
            child->setStrokeOpacity(origStrokeOpacity);
            child->setFillOpacity(origFillOpacity);
        }
    }
}
