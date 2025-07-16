#include "stdafx.h"
#include "rapidxml.hpp"
#include "SVGParser.h"
#include "SVGLine.h"
#include "SVGRect.h"
#include "SVGCircle.h"
#include "SVGText.h"
#include "SVGEllipse.h"
#include "SVGPolyline.h"
#include "SVGPolygon.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <regex>
#include <memory>

#include <windows.h>
#include <objidl.h>
#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")

using namespace Gdiplus;
using namespace rapidxml;

static Gdiplus::Color parseColor(xml_attribute<>* attr) {
    if (!attr) return Gdiplus::Color(0, 0, 0);
    std::string value = attr->value();
    int r = 0, g = 0, b = 0;
    if (sscanf_s(value.c_str(), "rgb(%d,%d,%d)", &r, &g, &b) == 3) {
        return Gdiplus::Color(r, g, b);
    }
    return Gdiplus::Color(0, 0, 0);
}

static float parseOpacity(xml_attribute<>* attr) {
    if (!attr) return 1.0f;
    return std::stof(attr->value());
}

static std::vector<Gdiplus::Point> parsePoints(const std::string& pointStr) {
    std::vector<Gdiplus::Point> points;
    std::regex rgx(R"((\d+),(\d+))");
    auto begin = std::sregex_iterator(pointStr.begin(), pointStr.end(), rgx);
    auto end = std::sregex_iterator();
    for (auto i = begin; i != end; ++i) {
        int x = std::stoi((*i)[1]);
        int y = std::stoi((*i)[2]);
        points.emplace_back(x, y);
    }
    return points;
}

SVGParseResult SVGParser::parse(const std::string& filename) {
    SVGParseResult result;

    std::ifstream file(filename);
    if (!file) {
        std::cerr << "Failed to open file: " << filename << "\n";
        return result;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();
    std::vector<char> xml_copy(content.begin(), content.end());
    xml_copy.push_back('\0');

    xml_document<> doc;
    doc.parse<0>(&xml_copy[0]);

    xml_node<>* svg = doc.first_node("svg");
    if (!svg) {
        std::cerr << "No <svg> element found.\n";
        return result;
    }

    auto makeColor = [&](xml_attribute<>* attr, float opacity = 1.0f) -> Gdiplus::Color* {
        std::string value = attr ? attr->value() : "";
        int r = 0, g = 0, b = 0;
        if (sscanf_s(value.c_str(), "rgb(%d,%d,%d)", &r, &g, &b) != 3) {
            r = g = b = 0;
        }
        BYTE alpha = static_cast<BYTE>(opacity * 255);
        result.colorPool.push_back(std::make_unique<Gdiplus::Color>(alpha, r, g, b));
        return result.colorPool.back().get();
        };

    for (xml_node<>* node = svg->first_node(); node; node = node->next_sibling()) {
        std::string tag = node->name();

        if (tag == "line") {
            int x1 = std::stoi(node->first_attribute("x1")->value());
            int y1 = std::stoi(node->first_attribute("y1")->value());
            int x2 = std::stoi(node->first_attribute("x2")->value());
            int y2 = std::stoi(node->first_attribute("y2")->value());
            int strokeW = node->first_attribute("stroke-width") ? std::stoi(node->first_attribute("stroke-width")->value()) : 1;
            float strokeOp = parseOpacity(node->first_attribute("stroke-opacity"));
            auto stroke = makeColor(node->first_attribute("stroke"), strokeOp);

            result.elements.push_back(std::make_unique<SVGLine>(x1, y1, x2, y2, stroke, strokeW));
        }
        else if (tag == "rect") {
            int x = std::stoi(node->first_attribute("x")->value());
            int y = std::stoi(node->first_attribute("y")->value());
            int w = std::stoi(node->first_attribute("width")->value());
            int h = std::stoi(node->first_attribute("height")->value());

            float fillOp = parseOpacity(node->first_attribute("fill-opacity"));
            float strokeOp = parseOpacity(node->first_attribute("stroke-opacity"));

            auto fill = makeColor(node->first_attribute("fill"), fillOp);
            auto stroke = makeColor(node->first_attribute("stroke"), strokeOp);
            int strokeW = node->first_attribute("stroke-width") ? std::stoi(node->first_attribute("stroke-width")->value()) : 1;

            result.elements.push_back(std::make_unique<SVGRect>(x, y, w, h, fill, stroke, strokeW));
        }
        else if (tag == "circle") {
            int cx = std::stoi(node->first_attribute("cx")->value());
            int cy = std::stoi(node->first_attribute("cy")->value());
            int r = std::stoi(node->first_attribute("r")->value());

            float fillOp = parseOpacity(node->first_attribute("fill-opacity"));
            float strokeOp = parseOpacity(node->first_attribute("stroke-opacity"));

            auto fill = makeColor(node->first_attribute("fill"), fillOp);
            auto stroke = makeColor(node->first_attribute("stroke"), strokeOp);
            int strokeW = node->first_attribute("stroke-width") ? std::stoi(node->first_attribute("stroke-width")->value()) : 1;

            result.elements.push_back(std::make_unique<SVGCircle>(cx, cy, r, fill, stroke, strokeW));
        }
        else if (tag == "text") {
            int x = std::stoi(node->first_attribute("x")->value());
            int y = std::stoi(node->first_attribute("y")->value());
            int fontSize = node->first_attribute("font-size") ? std::stoi(node->first_attribute("font-size")->value()) : 16;
            auto fill = makeColor(node->first_attribute("fill"));
            std::string content = node->value();

            result.elements.push_back(std::make_unique<SVGText>(x, y, fontSize, fill, content));
        }
        else if (tag == "ellipse") {
            int cx = std::stoi(node->first_attribute("cx")->value());
            int cy = std::stoi(node->first_attribute("cy")->value());
            int rx = std::stoi(node->first_attribute("rx")->value());
            int ry = std::stoi(node->first_attribute("ry")->value());

            float fillOp = parseOpacity(node->first_attribute("fill-opacity"));
            float strokeOp = parseOpacity(node->first_attribute("stroke-opacity"));

            auto fill = makeColor(node->first_attribute("fill"), fillOp);
            auto stroke = makeColor(node->first_attribute("stroke"), strokeOp);
            int strokeW = node->first_attribute("stroke-width") ? std::stoi(node->first_attribute("stroke-width")->value()) : 1;

            result.elements.push_back(std::make_unique<SVGEllipse>(cx, cy, rx, ry, fill, stroke, strokeW));
        }
        else if (tag == "polyline") {
            std::vector<Gdiplus::Point> pts = parsePoints(node->first_attribute("points")->value());

            float fillOp = parseOpacity(node->first_attribute("fill-opacity"));
            float strokeOp = parseOpacity(node->first_attribute("stroke-opacity"));

            auto fill = makeColor(node->first_attribute("fill"), fillOp);
            auto stroke = makeColor(node->first_attribute("stroke"), strokeOp);
            int strokeW = node->first_attribute("stroke-width") ? std::stoi(node->first_attribute("stroke-width")->value()) : 1;

            result.elements.push_back(std::make_unique<SVGPolyline>(pts, fill, stroke, strokeW));
        }
        else if (tag == "polygon") {
            std::vector<Gdiplus::Point> pts = parsePoints(node->first_attribute("points")->value());

            float fillOp = parseOpacity(node->first_attribute("fill-opacity"));
            float strokeOp = parseOpacity(node->first_attribute("stroke-opacity"));

            auto fill = makeColor(node->first_attribute("fill"), fillOp);
            auto stroke = makeColor(node->first_attribute("stroke"), strokeOp);
            int strokeW = node->first_attribute("stroke-width") ? std::stoi(node->first_attribute("stroke-width")->value()) : 1;

            result.elements.push_back(std::make_unique<SVGPolygon>(pts, fill, stroke, strokeW));
        }
    }

    return result;
}

