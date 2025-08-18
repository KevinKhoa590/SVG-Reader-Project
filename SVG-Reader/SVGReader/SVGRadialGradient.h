#pragma once

#include "SVGGradient.h"
#include <string>

class SVGRadialGradient : public SVGGradient {
public:
    SVGRadialGradient();
    std::string cx, cy, r, fx, fy;
};
