#pragma once

#include "SVGGradient.h"
#include <string>

class SVGLinearGradient : public SVGGradient {
public:
    SVGLinearGradient();
    std::string x1, y1, x2, y2;
    virtual SimpleColor sampleAt(double t) const override;
};
