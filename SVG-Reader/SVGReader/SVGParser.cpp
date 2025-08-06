// SVGParser.cpp
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
#include "SVGPath.h"
#include "SVGGroup.h"
#include "helper.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <regex>
#include <memory>
#include <functional>

#include <windows.h>
#include <objidl.h>
#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")

using namespace Gdiplus;
using namespace rapidxml;

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

    std::function<void(xml_node<>*, SVGGroup*)> parseNode;
    parseNode = [&](xml_node<>* node, SVGGroup* parentGroup) {
        std::string tag = node->name();

        auto addTo = [&](std::unique_ptr<SVGElement> el) {
            if (parentGroup)
                parentGroup->addElement(std::move(el));
            else
                result.elements.push_back(std::move(el));
            };

        auto applyAttributes = [](SVGElement* element, xml_node<>* node) {
            if (auto attr = node->first_attribute("transform"))
                element->setTransform(std::wstring(attr->value(), attr->value() + strlen(attr->value())));
            if (auto attr = node->first_attribute("stroke"))
                element->setStroke(std::wstring(attr->value(), attr->value() + strlen(attr->value())));
            if (auto attr = node->first_attribute("fill"))
                element->setFill(std::wstring(attr->value(), attr->value() + strlen(attr->value())));
            if (auto attr = node->first_attribute("stroke-width"))
                element->setStrokeWidth(std::wstring(attr->value(), attr->value() + strlen(attr->value())));
            if (auto attr = node->first_attribute("stroke-opacity"))
                element->setStrokeOpacity(std::wstring(attr->value(), attr->value() + strlen(attr->value())));
            if (auto attr = node->first_attribute("fill-opacity"))
                element->setFillOpacity(std::wstring(attr->value(), attr->value() + strlen(attr->value())));
            };

        if (tag == "line") {
            int x1 = std::stoi(node->first_attribute("x1")->value());
            int y1 = std::stoi(node->first_attribute("y1")->value());
            int x2 = std::stoi(node->first_attribute("x2")->value());
            int y2 = std::stoi(node->first_attribute("y2")->value());
            int strokeW = node->first_attribute("stroke-width") ? std::stoi(node->first_attribute("stroke-width")->value()) : 1;
            float strokeOp = parseOpacity(node->first_attribute("stroke-opacity"));
            auto stroke = makeColor(node->first_attribute("stroke"), strokeOp);
            addTo(std::make_unique<SVGLine>(x1, y1, x2, y2, stroke, strokeW));
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

            addTo(std::make_unique<SVGRect>(x, y, w, h, fill, stroke, strokeW));
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

            addTo(std::make_unique<SVGCircle>(cx, cy, r, fill, stroke, strokeW));
        }
        else if (tag == "text") {
            int x = std::stoi(node->first_attribute("x")->value());
            int y = std::stoi(node->first_attribute("y")->value());
            int fontSize = node->first_attribute("font-size") ? std::stoi(node->first_attribute("font-size")->value()) : 16;
            auto fill = makeColor(node->first_attribute("fill"));
            std::string content = node->value();

            addTo(std::make_unique<SVGText>(x, y, fontSize, fill, content));
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

            addTo(std::make_unique<SVGEllipse>(cx, cy, rx, ry, fill, stroke, strokeW));
        }
        else if (tag == "polyline") {
            std::vector<Gdiplus::Point> pts = parsePoints(node->first_attribute("points")->value());

            float fillOp = parseOpacity(node->first_attribute("fill-opacity"));
            float strokeOp = parseOpacity(node->first_attribute("stroke-opacity"));

            auto fill = makeColor(node->first_attribute("fill"), fillOp);
            auto stroke = makeColor(node->first_attribute("stroke"), strokeOp);
            int strokeW = node->first_attribute("stroke-width") ? std::stoi(node->first_attribute("stroke-width")->value()) : 1;

            addTo(std::make_unique<SVGPolyline>(pts, fill, stroke, strokeW));
        }
        else if (tag == "polygon") {
            std::vector<Gdiplus::Point> pts = parsePoints(node->first_attribute("points")->value());

            float fillOp = parseOpacity(node->first_attribute("fill-opacity"));
            float strokeOp = parseOpacity(node->first_attribute("stroke-opacity"));

            auto fill = makeColor(node->first_attribute("fill"), fillOp);
            auto stroke = makeColor(node->first_attribute("stroke"), strokeOp);
            int strokeW = node->first_attribute("stroke-width") ? std::stoi(node->first_attribute("stroke-width")->value()) : 1;

            addTo(std::make_unique<SVGPolygon>(pts, fill, stroke, strokeW));
        }
        else if (tag == "path") {
            auto path = std::make_unique<SVGPath>();
            for (xml_attribute<>* attr = node->first_attribute(); attr; attr = attr->next_attribute()) {
                path->setAttribute(attr);
            }
            addTo(std::move(path));
        }
        else if (tag == "g") {
            std::unique_ptr<SVGGroup> group = std::make_unique<SVGGroup>();

            // Apply transform attribute
            if (auto attr = node->first_attribute("transform")) {
                std::wstring t(attr->value(), attr->value() + strlen(attr->value()));
                group->setTransform(parseTransform(t));
            }

            // Optional: Apply inherited styles to group (pass to children if needed)
            applyAttributes(group.get(), node);

            for (xml_node<>* child = node->first_node(); child; child = child->next_sibling()) {
                parseNode(child, group.get());
            }

            addTo(std::move(group));
        }
     };

    for (xml_node<>* node = svg->first_node(); node; node = node->next_sibling()) {
        parseNode(node, nullptr);
    }

    return result;
}
