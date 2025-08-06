#pragma once

#include "SVGElement.h"
#include <vector>
#include <memory>

namespace Gdiplus {
    class Graphics;
    class Matrix;
}

class SVGGroup : public SVGElement {
private:
    std::vector<std::unique_ptr<SVGElement>> children;
    std::unique_ptr<Gdiplus::Matrix> transform; // smart pointer

public:
    SVGGroup();
    ~SVGGroup();

    void addElement(std::unique_ptr<SVGElement> child);
    void setTransform(Gdiplus::Matrix* matrix); // takes ownership
    void draw(Gdiplus::Graphics& g) override;
    void elementRender(Gdiplus::Graphics& g); // apply transform
};
