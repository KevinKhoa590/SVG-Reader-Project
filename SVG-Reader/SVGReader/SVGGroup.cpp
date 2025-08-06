#include "stdafx.h"
#include "SVGGroup.h"
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
        g.MultiplyTransform(transform.get(), MatrixOrderPrepend);
    }

    draw(g); // draw without reapplying transform

    g.Restore(state); // restore graphics state
}

void SVGGroup::draw(Graphics& g) {
    for (auto& child : children) {
        if (child)
            child->elementRender(g); // important: always use elementRender
    }
}
