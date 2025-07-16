#pragma once

#include <vector>
#include <string>
#include <memory>
#include "SVGElement.h"

namespace Gdiplus {
    class Color;
}

struct SVGParseResult {
    std::vector<std::unique_ptr<SVGElement>> elements;
    std::vector<std::unique_ptr<Gdiplus::Color>> colorPool;
};

class SVGParser {
public:
    static SVGParseResult parse(const std::string& filename);
};
