
#pragma once

#include "rapidxml.hpp"
#include "SVGElement.h"
#include "SVGGroup.h"
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>

using namespace rapidxml;

namespace Gdiplus {
    class Color;
}

struct SVGParseResult {
    std::vector<std::unique_ptr<SVGElement>> elements;
    // colorPool owns Gdiplus::Color objects created while parsing (for fills/strokes and gradient fallback colors)
    std::vector<std::unique_ptr<Gdiplus::Color>> colorPool;

    // Gradients storage: own all parsed gradients here
    std::vector<std::unique_ptr<SVGGradient>> gradients;

    // Quick lookup map id -> raw pointer (raw pointer points into the unique_ptr above)
    std::unordered_map<std::string, SVGGradient*> gradientsById;
};

class SVGParser {
public:
    static void parseDefs(xml_node<>* defsNode, SVGParseResult& result);
    static void parseNode(xml_node<>* node, SVGGroup* parentGroup, SVGParseResult& result);
    static SVGParseResult parse(const std::string& filename);
};
