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
#include "SVGGradient.h"
#include "SVGLinearGradient.h"
#include "SVGRadialGradient.h"
#include "helper.h"

#include <cstring>
#include <fstream>
#include <sstream>
#include <iostream>
#include <regex>
#include <memory>
#include <algorithm>
#include <unordered_map>
#include <tuple>
#include <vector>

#include <windows.h>
#include <objidl.h>
#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")

using namespace Gdiplus;
using namespace rapidxml;

// Parse <defs> children (linearGradient/radialGradient)
void SVGParser::parseDefs(xml_node<>* defsNode, SVGParseResult& result) {
    if (!defsNode) return;

    for (xml_node<>* child = defsNode->first_node(); child; child = child->next_sibling()) {
        std::string tag = child->name();
        if (tag == "linearGradient") {
            std::unique_ptr<SVGLinearGradient> lg = std::make_unique<SVGLinearGradient>();
            if (auto a = child->first_attribute("id")) lg->id = std::string(a->value());
            if (auto a = child->first_attribute("x1")) lg->x1 = std::string(a->value());
            if (auto a = child->first_attribute("y1")) lg->y1 = std::string(a->value());
            if (auto a = child->first_attribute("x2")) lg->x2 = std::string(a->value());
            if (auto a = child->first_attribute("y2")) lg->y2 = std::string(a->value());
            if (auto a = child->first_attribute("gradientUnits")) {
                std::string u = std::string(a->value());
                if (u == "userSpaceOnUse") lg->units = SVGGradient::Units::USER_SPACE_ON_USE;
                else lg->units = SVGGradient::Units::OBJECT_BOUNDING_BOX;
            }
            if (auto a = child->first_attribute("gradientTransform")) lg->gradientTransform = std::string(a->value());

            if (auto a = child->first_attribute("href")) {
                std::string href = a->value();
                if (!href.empty() && href[0] == '#') href = href.substr(1);
                lg->href = href;
            }
            else if (auto a2 = child->first_attribute("xlink:href")) {
                std::string href = a2->value();
                if (!href.empty() && href[0] == '#') href = href.substr(1);
                lg->href = href;
            }

            for (xml_node<>* stop = child->first_node("stop"); stop; stop = stop->next_sibling("stop")) {
                GradientStop gs = parseStopNode(stop, result);
                lg->addStop(gs);
            }
            
            SVGGradient* raw = lg.get();
            result.gradients.push_back(std::move(lg));
            if (!raw->id.empty()) {
                result.gradientsById[raw->id] = raw;
            }
        }
        else if (tag == "radialGradient") {
            std::unique_ptr<SVGRadialGradient> rg = std::make_unique<SVGRadialGradient>();
            if (auto a = child->first_attribute("id")) rg->id = std::string(a->value());
            if (auto a = child->first_attribute("cx")) rg->cx = std::string(a->value());
            if (auto a = child->first_attribute("cy")) rg->cy = std::string(a->value());
            if (auto a = child->first_attribute("r"))  rg->r = std::string(a->value());
            if (auto a = child->first_attribute("fx")) rg->fx = std::string(a->value());
            if (auto a = child->first_attribute("fy")) rg->fy = std::string(a->value());
            if (auto a = child->first_attribute("gradientUnits")) {
                std::string u = std::string(a->value());
                if (u == "userSpaceOnUse") rg->units = SVGGradient::Units::USER_SPACE_ON_USE;
                else rg->units = SVGGradient::Units::OBJECT_BOUNDING_BOX;
            }
            if (auto a = child->first_attribute("gradientTransform")) rg->gradientTransform = std::string(a->value());

            if (auto a = child->first_attribute("href")) {
                std::string href = a->value();
                if (!href.empty() && href[0] == '#') href = href.substr(1);
                rg->gradientTransform = std::string("href:") + href;
            }
            else if (auto a2 = child->first_attribute("xlink:href")) {
                std::string href = a2->value();
                if (!href.empty() && href[0] == '#') href = href.substr(1);
                rg->gradientTransform = std::string("href:") + href;
            }

            for (xml_node<>* stop = child->first_node("stop"); stop; stop = stop->next_sibling("stop")) {
                GradientStop gs = parseStopNode(stop, result);
                rg->addStop(gs);
            }

            SVGGradient* raw = rg.get();
            result.gradients.push_back(std::move(rg));
            if (!raw->id.empty()) {
                result.gradientsById[raw->id] = raw;
            }
        }
        else {
            // ignore other defs
        }
    }

    // Resolve href copies
    for (auto& u : result.gradients) {
        SVGGradient* g = u.get();
        if (!g->href.empty()) {
            auto it = result.gradientsById.find(g->href);
            if (it != result.gradientsById.end()) {
                SVGGradient* refg = it->second;
                if (g->getStops().empty()) {
                    for (const GradientStop& s : refg->getStops()) g->addStop(s);
                }
            }
            g->gradientTransform.clear();
        }
    }
}

void SVGParser::parseNode(xml_node<>* node, SVGGroup* parentGroup, SVGParseResult& result) {
    if (!node) return;
    std::string tag = node->name();

    auto addTo = [&](std::unique_ptr<SVGElement> el) {
        if (!el) return;
        if (parentGroup) parentGroup->addElement(std::move(el));
        else result.elements.push_back(std::move(el));
        };

    // line
    if (tag == "line") {
        xml_attribute<>* a_x1 = getAttr(node, "x1");
        xml_attribute<>* a_y1 = getAttr(node, "y1");
        xml_attribute<>* a_x2 = getAttr(node, "x2");
        xml_attribute<>* a_y2 = getAttr(node, "y2");
        if (!a_x1 || !a_y1 || !a_x2 || !a_y2) return;

        int x1 = parseIntAttr(node, "x1", 0);
        int y1 = parseIntAttr(node, "y1", 0);
        int x2 = parseIntAttr(node, "x2", 0);
        int y2 = parseIntAttr(node, "y2", 0);

        std::string strokeStr = effectiveAttrString(node, "stroke", parentGroup);
        float strokeOp = effectiveOpacity(node, "stroke-opacity", parentGroup, 1.0f);
        int strokeW = effectiveStrokeWidth(node, parentGroup, 1);

        SVGGradient* strokeGrad = nullptr;
        auto strokeColor = resolvePaint(result, strokeStr, strokeOp, strokeGrad);

        auto el = std::make_unique<SVGLine>(x1, y1, x2, y2, strokeColor, strokeW);
        if (strokeGrad) el->setStrokeGradient(strokeGrad);

        applyAttributesIfPresent(el.get(), node);
        addTo(std::move(el));
        return;
    }

    // rect
    if (tag == "rect") {
        xml_attribute<>* a_x = getAttr(node, "x");
        xml_attribute<>* a_y = getAttr(node, "y");
        xml_attribute<>* a_w = getAttr(node, "width");
        xml_attribute<>* a_h = getAttr(node, "height");
        if (!a_x || !a_y || !a_w || !a_h) return;

        int x = parseIntAttr(node, "x", 0);
        int y = parseIntAttr(node, "y", 0);
        int w = parseIntAttr(node, "width", 0);
        int h = parseIntAttr(node, "height", 0);

        std::string fillStr = effectiveAttrString(node, "fill", parentGroup);
        float fillOp = effectiveOpacity(node, "fill-opacity", parentGroup, 1.0f);

        std::string strokeStr = effectiveAttrString(node, "stroke", parentGroup);
        float strokeOp = effectiveOpacity(node, "stroke-opacity", parentGroup, 1.0f);

        int strokeW = effectiveStrokeWidth(node, parentGroup, 1);

        SVGGradient* fillGrad = nullptr;
        SVGGradient* strokeGrad = nullptr;
        auto fillColor = resolvePaint(result, fillStr, fillOp, fillGrad);
        auto strokeColor = resolvePaint(result, strokeStr, strokeOp, strokeGrad);

        auto el = std::make_unique<SVGRect>(x, y, w, h, fillColor, strokeColor, strokeW);
        if (fillGrad) el->setFillGradient(fillGrad);
        if (strokeGrad) el->setStrokeGradient(strokeGrad);
        applyAttributesIfPresent(el.get(), node);
        addTo(std::move(el));
        return;
    }

    // circle
    if (tag == "circle") {
        xml_attribute<>* a_cx = getAttr(node, "cx");
        xml_attribute<>* a_cy = getAttr(node, "cy");
        xml_attribute<>* a_r = getAttr(node, "r");
        if (!a_cx || !a_cy || !a_r) return;

        int cx = parseIntAttr(node, "cx", 0);
        int cy = parseIntAttr(node, "cy", 0);
        int r = parseIntAttr(node, "r", 0);

        std::string fillStr = effectiveAttrString(node, "fill", parentGroup);
        float fillOp = effectiveOpacity(node, "fill-opacity", parentGroup, 1.0f);

        std::string strokeStr = effectiveAttrString(node, "stroke", parentGroup);
        float strokeOp = effectiveOpacity(node, "stroke-opacity", parentGroup, 1.0f);

        int strokeW = effectiveStrokeWidth(node, parentGroup, 1);

        SVGGradient* fillGrad = nullptr;
        SVGGradient* strokeGrad = nullptr;
        auto fillColor = resolvePaint(result, fillStr, fillOp, fillGrad);
        auto strokeColor = resolvePaint(result, strokeStr, strokeOp, strokeGrad);

        auto el = std::make_unique<SVGCircle>(cx, cy, r, fillColor, strokeColor, strokeW);
        if (fillGrad) el->setFillGradient(fillGrad);
        if (strokeGrad) el->setStrokeGradient(strokeGrad);
        applyAttributesIfPresent(el.get(), node);
        addTo(std::move(el));
        return;
    }

    // text
    if (tag == "text") {
        xml_attribute<>* a_x = getAttr(node, "x");
        xml_attribute<>* a_y = getAttr(node, "y");
        if (!a_x || !a_y) return;

        int x = parseIntAttr(node, "x", 0);
        int y = parseIntAttr(node, "y", 0);
        int fontSize = parseIntAttr(node, "font-size", 16);

        std::string fillStr = effectiveAttrString(node, "fill", parentGroup);
        float fillOp = effectiveOpacity(node, "fill-opacity", parentGroup, 1.0f);

        SVGGradient* fillGrad = nullptr;
        auto fillColor = resolvePaint(result, fillStr, fillOp, fillGrad);
        std::string content = node->value();

        auto el = std::make_unique<SVGText>(x, y, fontSize, fillColor, content);
        if (fillGrad) el->setFillGradient(fillGrad);
        applyAttributesIfPresent(el.get(), node);
        addTo(std::move(el));
        return;
    }

    // ellipse
    if (tag == "ellipse") {
        xml_attribute<>* a_cx = getAttr(node, "cx");
        xml_attribute<>* a_cy = getAttr(node, "cy");
        xml_attribute<>* a_rx = getAttr(node, "rx");
        xml_attribute<>* a_ry = getAttr(node, "ry");
        if (!a_cx || !a_cy || !a_rx || !a_ry) return;

        int cx = parseIntAttr(node, "cx", 0);
        int cy = parseIntAttr(node, "cy", 0);
        int rx = parseIntAttr(node, "rx", 0);
        int ry = parseIntAttr(node, "ry", 0);

        std::string fillStr = effectiveAttrString(node, "fill", parentGroup);
        float fillOp = effectiveOpacity(node, "fill-opacity", parentGroup, 1.0f);

        std::string strokeStr = effectiveAttrString(node, "stroke", parentGroup);
        float strokeOp = effectiveOpacity(node, "stroke-opacity", parentGroup, 1.0f);

        int strokeW = effectiveStrokeWidth(node, parentGroup, 1);

        SVGGradient* fillGrad = nullptr;
        SVGGradient* strokeGrad = nullptr;
        auto fillColor = resolvePaint(result, fillStr, fillOp, fillGrad);
        auto strokeColor = resolvePaint(result, strokeStr, strokeOp, strokeGrad);

        auto el = std::make_unique<SVGEllipse>(cx, cy, rx, ry, fillColor, strokeColor, strokeW);
        if (fillGrad) el->setFillGradient(fillGrad);
        if (strokeGrad) el->setStrokeGradient(strokeGrad);
        applyAttributesIfPresent(el.get(), node);
        addTo(std::move(el));
        return;
    }

    // polyline
    if (tag == "polyline") {
        if (!getAttr(node, "points")) return;
        std::vector<Gdiplus::Point> pts = parsePoints(getAttrValue(node, "points"));

        std::string fillStr = effectiveAttrString(node, "fill", parentGroup);
        float fillOp = effectiveOpacity(node, "fill-opacity", parentGroup, 1.0f);

        std::string strokeStr = effectiveAttrString(node, "stroke", parentGroup);
        float strokeOp = effectiveOpacity(node, "stroke-opacity", parentGroup, 1.0f);

        int strokeW = effectiveStrokeWidth(node, parentGroup, 1);

        SVGGradient* fillGrad = nullptr;
        SVGGradient* strokeGrad = nullptr;
        auto fillColor = resolvePaint(result, fillStr, fillOp, fillGrad);
        auto strokeColor = resolvePaint(result, strokeStr, strokeOp, strokeGrad);

        auto el = std::make_unique<SVGPolyline>(pts, fillColor, strokeColor, strokeW);
        if (fillGrad) el->setFillGradient(fillGrad);
        if (strokeGrad) el->setStrokeGradient(strokeGrad);
        applyAttributesIfPresent(el.get(), node);
        addTo(std::move(el));
        return;
    }

    // polygon
    if (tag == "polygon") {
        if (!getAttr(node, "points")) return;
        std::vector<Gdiplus::Point> pts = parsePoints(getAttrValue(node, "points"));

        std::string fillStr = effectiveAttrString(node, "fill", parentGroup);
        float fillOp = effectiveOpacity(node, "fill-opacity", parentGroup, 1.0f);

        std::string strokeStr = effectiveAttrString(node, "stroke", parentGroup);
        float strokeOp = effectiveOpacity(node, "stroke-opacity", parentGroup, 1.0f);

        int strokeW = effectiveStrokeWidth(node, parentGroup, 1);

        SVGGradient* fillGrad = nullptr;
        SVGGradient* strokeGrad = nullptr;
        auto fillColor = resolvePaint(result, fillStr, fillOp, fillGrad);
        auto strokeColor = resolvePaint(result, strokeStr, strokeOp, strokeGrad);

        auto el = std::make_unique<SVGPolygon>(pts, fillColor, strokeColor, strokeW);
        if (fillGrad) el->setFillGradient(fillGrad);
        if (strokeGrad) el->setStrokeGradient(strokeGrad);
        applyAttributesIfPresent(el.get(), node);
        addTo(std::move(el));
        return;
    }

    // path
    if (tag == "path") {
        auto path = std::make_unique<SVGPath>();
        for (xml_attribute<>* attr = node->first_attribute(); attr; attr = attr->next_attribute()) {
            path->setAttribute(attr);
        }

        std::string fillStr = effectiveAttrString(node, "fill", parentGroup);
        float fillOp = effectiveOpacity(node, "fill-opacity", parentGroup, 1.0f);
        std::string strokeStr = effectiveAttrString(node, "stroke", parentGroup);
        float strokeOp = effectiveOpacity(node, "stroke-opacity", parentGroup, 1.0f);
        int strokeW = effectiveStrokeWidth(node, parentGroup, 1);

        SVGGradient* fillGrad = nullptr;
        SVGGradient* strokeGrad = nullptr;
        auto fillColor = resolvePaint(result, fillStr, fillOp, fillGrad);
        auto strokeColor = resolvePaint(result, strokeStr, strokeOp, strokeGrad);

        applyAttributesIfPresent(path.get(), node);
        if (fillGrad) path->setFillGradient(fillGrad);
        if (strokeGrad) path->setStrokeGradient(strokeGrad);

        addTo(std::move(path));
        return;
    }

    // group
    if (tag == "g") {
        auto group = std::make_unique<SVGGroup>();

        if (auto attr = getAttr(node, "transform")) {
            std::wstring t(attr->value(), attr->value() + strlen(attr->value()));
            group->setTransform(parseTransform(t));
        }

        applyAttributesIfPresent(group.get(), node);

        for (xml_node<>* child = node->first_node(); child; child = child->next_sibling()) {
            parseNode(child, group.get(), result);
        }

        addTo(std::move(group));
        return;
    }
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
    try {
        doc.parse<0>(&xml_copy[0]);
    }
    catch (const std::exception& ex) {
        std::cerr << "XML parse error: " << ex.what() << "\n";
        return result;
    }

    xml_node<>* svg = doc.first_node("svg");
    if (!svg) {
        std::cerr << "No <svg> element found.\n";
        return result;
    }

    // First pass: parse defs (gradients)
    for (xml_node<>* node = svg->first_node(); node; node = node->next_sibling()) {
        std::string tag = node->name();
        if (tag == "defs") {
            parseDefs(node, result);
        }
    }

    // Second pass: parse everything else
    for (xml_node<>* node = svg->first_node(); node; node = node->next_sibling()) {
        std::string tag = node->name();
        if (tag == "defs") continue;
        parseNode(node, nullptr, result);
    }

    return result;
}
