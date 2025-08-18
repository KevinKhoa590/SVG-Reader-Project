#include "stdafx.h"
#include "SVGLinearGradient.h"
#include <algorithm>

SVGLinearGradient::SVGLinearGradient() : SVGGradient() {}

SimpleColor SVGLinearGradient::sampleAt(double t) const {
    return SVGGradient::sampleAt(t);
}
