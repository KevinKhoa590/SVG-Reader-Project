#include "stdafx.h"
#include "SVGElement.h"
#include <windows.h>
#include <objidl.h>
#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")

using namespace Gdiplus;

void SVGElement::elementRender(Gdiplus::Graphics& g) {
    GraphicsState state = g.Save();
    g.MultiplyTransform(transform.getMatrix());
    draw(g);
    g.Restore(state);
}
