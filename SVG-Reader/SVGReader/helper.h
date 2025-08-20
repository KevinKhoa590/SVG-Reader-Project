#include "stdafx.h"
#include "rapidxml.hpp"
#include "helper.h"
#include <windows.h>
#include <objidl.h>
#include <gdiplus.h>
#include <regex>
#include <sstream>
#include <cctype>
#include <map>
#include <unordered_map>
#pragma comment(lib, "gdiplus.lib")
using namespace Gdiplus;
using namespace rapidxml;

Gdiplus::Color parseColor(xml_attribute<>* attr) {
    if (!attr) return Gdiplus::Color(0, 0, 0);

    std::string value = attr->value();

    // Handle rgb(r,g,b)
    int r, g, b;
    if (sscanf_s(value.c_str(), "rgb(%d,%d,%d)", &r, &g, &b) == 3) {
        return Gdiplus::Color(r, g, b);
    }

    // Handle hex colors like #rrggbb
    if (value[0] == '#' && value.length() == 7) {
        int hex;
        if (sscanf_s(value.c_str() + 1, "%x", &hex) == 1) {
            r = (hex >> 16) & 0xFF;
            g = (hex >> 8) & 0xFF;
            b = hex & 0xFF;
            return Gdiplus::Color(r, g, b);
        }
    }

    // Handle some basic named colors
    static const std::map<std::string, Gdiplus::Color> namedColors = {
        {"black", Gdiplus::Color(0,0,0)},
        {"white", Gdiplus::Color(255,255,255)},
        {"red", Gdiplus::Color(255,0,0)},
        {"green", Gdiplus::Color(0,128,0)},
        {"blue", Gdiplus::Color(0,0,255)},
        {"yellow", Gdiplus::Color(255,255,0)},
        {"gray", Gdiplus::Color(128,128,128)},
        {"grey", Gdiplus::Color(128,128,128)},
    };

    auto it = namedColors.find(value);
    if (it != namedColors.end()) {
        return it->second;
    }

    return Gdiplus::Color(0, 0, 0); // fallback black
}

float parseOpacity(xml_attribute<>* attr) {
    if (!attr) return 1.0f;
    return std::stof(attr->value());
}

std::vector<Gdiplus::Point> parsePoints(const std::string& pointStr) {
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

Gdiplus::Matrix* parseTransform(const std::wstring& transformStr) {
    auto matrix = new Gdiplus::Matrix();

    std::wregex commandRe(LR"((\w+)\s*\(([^)]*)\))");
    std::wsmatch match;
    std::wstring input = transformStr;

    while (std::regex_search(input, match, commandRe)) {
        std::wstring cmd = match[1];
        std::wstring paramsStr = match[2];

        // Parse parameters
        std::wstringstream ss(paramsStr);
        std::vector<float> params;
        std::wstring token;
        while (std::getline(ss, token, L',')) {
            std::wstringstream inner(token);
            float value;
            while (inner >> value) {
                params.push_back(value);
            }
        }

        if (cmd == L"translate") {
            float dx = params.size() > 0 ? params[0] : 0.0f;
            float dy = params.size() > 1 ? params[1] : 0.0f;
            matrix->Translate(dx, dy, Gdiplus::MatrixOrderAppend);
        }
        else if (cmd == L"scale") {
            float sx = params.size() > 0 ? params[0] : 1.0f;
            float sy = params.size() > 1 ? params[1] : sx;
            matrix->Scale(sx, sy, Gdiplus::MatrixOrderAppend);
        }
        else if (cmd == L"rotate") {
            float angle = params.size() > 0 ? params[0] : 0.0f;
            if (params.size() == 3) {
                float cx = params[1], cy = params[2];
                matrix->Translate(-cx, -cy, Gdiplus::MatrixOrderAppend);
                matrix->Rotate(angle, Gdiplus::MatrixOrderAppend);
                matrix->Translate(cx, cy, Gdiplus::MatrixOrderAppend);
            }
            else {
                matrix->Rotate(angle, Gdiplus::MatrixOrderAppend);
            }
        }

        input = match.suffix();
    }

    return matrix;
}

// Safe attribute retrieval
xml_attribute<>* getAttr(xml_node<>* node, const char* name) {
    if (!node) return nullptr;
    return node->first_attribute(name);
}

std::string getAttrValue(xml_node<>* node, const char* name, const std::string& def) {
    xml_attribute<>* a = getAttr(node, name);
    if (!a) return def;
    return std::string(a->value());
}

// safe integer parsing with default
int parseIntAttr(xml_node<>* node, const char* name, int def) {
    xml_attribute<>* a = getAttr(node, name);
    if (!a) return def;
    try {
        return std::stoi(a->value());
    }
    catch (...) {
        return def;
    }
}

// hex parser
int hexDigit(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return 10 + (c - 'a');
    if (c >= 'A' && c <= 'F') return 10 + (c - 'A');
    return 0;
}

// clamp float to [0,1]
float clamp01(float v) {
    if (v < 0.f) return 0.f;
    if (v > 1.f) return 1.f;
    return v;
}

// Apply simple presentation attributes (transform, stroke, fill, widths, opacities) onto an element,
// but only if the attributes are present on the node
void applyAttributesIfPresent(SVGElement* element, xml_node<>* node) {
    if (!element || !node) return;
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
}

// trim (narrow)
std::string trim_s(const std::string& s) {
    size_t a = s.find_first_not_of(" \t\r\n");
    if (a == std::string::npos) return "";
    size_t b = s.find_last_not_of(" \t\r\n");
    return s.substr(a, b - a + 1);
}

// clamp 0..1
float clamp01f(float v) {
    if (v < 0.0f) return 0.0f;
    if (v > 1.0f) return 1.0f;
    return v;
}

// parse #rgb or #rrggbb; returns true if parsed
bool parseHexColor_local(const std::string& s, int& r, int& g, int& b) {
    if (s.size() == 4 && s[0] == '#') { // #rgb
        r = hexDigit(s[1]) * 17;
        g = hexDigit(s[2]) * 17;
        b = hexDigit(s[3]) * 17;
        return true;
    }
    if (s.size() == 7 && s[0] == '#') { // #rrggbb
        r = hexDigit(s[1]) * 16 + hexDigit(s[2]);
        g = hexDigit(s[3]) * 16 + hexDigit(s[4]);
        b = hexDigit(s[5]) * 16 + hexDigit(s[6]);
        return true;
    }
    return false;
}

// lowercase string
std::string toLowerStr(const std::string& s) {
    std::string out = s;
    std::transform(out.begin(), out.end(), out.begin(),
        [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return out;
}

// narrow wstring
std::string narrow(const std::wstring& ws) {
    std::string s;
    s.reserve(ws.size());
    for (wchar_t wc : ws) s.push_back(static_cast<char>(wc));
    return s;
}

// Parse opacity from a wstring
float parseOpacityFromWString(const std::wstring& w, float def) {
    if (w.empty()) return def;
    try {
        std::string s = narrow(w);
        return clamp01f(std::stof(s));
    }
    catch (...) {
        return def;
    }
}

// Make or reuse a Color in result.colorPool from a narrow string (supports named, #hex, rgb(...)).
Gdiplus::Color* makeColorFromString(SVGParseResult& result, const std::string& rawIn, float opacity) {
    std::string raw = trim_s(rawIn);
    if (raw.empty()) return nullptr;
    std::string low = toLowerStr(raw);

    // return transparent Color for "none"
    if (low == "none") {
        BYTE alpha = static_cast<BYTE>(clamp01f(opacity) * 255.0f);
        result.colorPool.push_back(std::make_unique<Gdiplus::Color>(alpha, 0, 0, 0));
        return result.colorPool.back().get();
    }

    // named color table
    static const std::unordered_map<std::string, std::tuple<int, int, int>> namedColors = {
        {"black",  {0,0,0}},
        {"white",  {255,255,255}},
        {"red",    {255,0,0}},
        {"green",  {0,128,0}},
        {"blue",   {0,0,255}},
        {"yellow", {255,255,0}},
        {"cyan",   {0,255,255}},
        {"magenta",{255,0,255}},
        {"gray",   {128,128,128}},
        {"grey",   {128,128,128}},
        {"orange", {255,165,0}},
        {"purple", {128,0,128}}
    };

    int r = 0, g = 0, b = 0;
    bool parsed = false;

    // hex
    if (parseHexColor_local(low, r, g, b)) {
        parsed = true;
    }
    else {
        // rgb(r,g,b)
        int rr, gg, bb;
        if (sscanf_s(raw.c_str(), "rgb(%d,%d,%d)", &rr, &gg, &bb) == 3) {
            r = rr; g = gg; b = bb;
            parsed = true;
        }
        else {
            auto it = namedColors.find(low);
            if (it != namedColors.end()) {
                std::tie(r, g, b) = it->second;
                parsed = true;
            }
        }
    }

    if (!parsed) {
        // unknown -> return nullptr
        return nullptr;
    }

    BYTE alpha = static_cast<BYTE>(clamp01f(opacity) * 255.0f);
    result.colorPool.push_back(std::make_unique<Gdiplus::Color>(alpha, r, g, b));
    return result.colorPool.back().get();
}

// Get effective attribute string (node attribute if present, otherwise group's value)
// returns narrow (std::string). If neither present, returns empty string.
std::string effectiveAttrString(xml_node<>* node, const char* attrName, SVGGroup* parentGroup) {
    if (node) {
        if (auto a = node->first_attribute(attrName))
            return std::string(a->value());
    }
    if (parentGroup) {
        if (strcmp(attrName, "fill") == 0) {
            std::wstring tmp = parentGroup->getFill();
            if (!tmp.empty()) return narrow(tmp);
        }
        if (strcmp(attrName, "stroke") == 0) {
            std::wstring tmp = parentGroup->getStroke();
            if (!tmp.empty()) return narrow(tmp);
        }
    }
    return "";
}

// Get effective opacity: node attribute first, else parentGroup's stored wstring value, else default
float effectiveOpacity(xml_node<>* node, const char* attrName, SVGGroup* parentGroup, float def) {
    if (node) {
        if (auto a = node->first_attribute(attrName)) {
            return parseOpacity(a);
        }
    }
    if (parentGroup) {
        if (strcmp(attrName, "fill-opacity") == 0) {
            return parseOpacityFromWString(parentGroup->getFillOpacity(), def);
        }
        if (strcmp(attrName, "stroke-opacity") == 0) {
            return parseOpacityFromWString(parentGroup->getStrokeOpacity(), def);
        }
    }
    return def;
}

// Get effective stroke-width: node attribute first, else parent's wstring, else default int
int effectiveStrokeWidth(xml_node<>* node, SVGGroup* parentGroup, int def) {
    if (node && node->first_attribute("stroke-width")) {
        try { return std::stoi(node->first_attribute("stroke-width")->value()); }
        catch (...) { return def; }
    }
    if (parentGroup) {
        std::wstring p = parentGroup->getStrokeWidth();
        if (!p.empty()) {
            try { return std::stoi(narrow(p)); }
            catch (...) {}
        }
    }
    return def;
}

// convert SimpleColor to Gdiplus::Color
Color simpleToGdi(const SimpleColor& sc) {
    return Color(static_cast<BYTE>(sc.a), sc.r, sc.g, sc.b);
}

// parse a coordinate string ("" or number or percentage). Defaults used if empty.
float parseCoordForLinear(const std::string& s, float defaultVal, const RectF& bounds, const SVGLinearGradient* lg, bool isX) {
    if (s.empty()) return defaultVal;

    std::string t = s;
    auto first = t.find_first_not_of(" \t\r\n");
    if (first != std::string::npos) t = t.substr(first, t.find_last_not_of(" \t\r\n") - first + 1);
    if (t.empty()) return defaultVal;

    if (t.back() == '%') {
        try {
            double v = std::stod(t.substr(0, t.size() - 1));
            double frac = v / 100.0;
            if (lg && lg->units == SVGGradient::Units::OBJECT_BOUNDING_BOX) {
                return isX ? bounds.X + bounds.Width * static_cast<float>(frac)
                    : bounds.Y + bounds.Height * static_cast<float>(frac);
            }
            else {
                return isX ? bounds.X + bounds.Width * static_cast<float>(frac)
                    : bounds.Y + bounds.Height * static_cast<float>(frac);
            }
        }
        catch (...) { return defaultVal; }
    }

    try {
        double v = std::stod(t);
        if (lg && lg->units == SVGGradient::Units::OBJECT_BOUNDING_BOX) {
            return isX ? bounds.X + bounds.Width * static_cast<float>(v)
                : bounds.Y + bounds.Height * static_cast<float>(v);
        }
        else {
            return static_cast<float>(v);
        }
    }
    catch (...) {
        return defaultVal;
    }
}

// Parse a coordinate for radial gradients (handles percentages vs user space)
float parseCoordForRadial(const std::string& s, float defaultVal, const RectF& bounds, const SVGGradient* grad, bool isX) {
    if (s.empty()) return defaultVal;
    std::string t = s;
    auto first = t.find_first_not_of(" \t\r\n");
    if (first != std::string::npos) t = t.substr(first, t.find_last_not_of(" \t\r\n") - first + 1);
    if (t.empty()) return defaultVal;

    if (t.back() == '%') {
        try {
            double v = std::stod(t.substr(0, t.size() - 1));
            double frac = v / 100.0;
            return isX ? bounds.X + bounds.Width * static_cast<float>(frac)
                : bounds.Y + bounds.Height * static_cast<float>(frac);
        }
        catch (...) { return defaultVal; }
    }

    try {
        double v = std::stod(t);
        if (grad && grad->units == SVGGradient::Units::OBJECT_BOUNDING_BOX) {
            return isX ? bounds.X + bounds.Width * static_cast<float>(v)
                : bounds.Y + bounds.Height * static_cast<float>(v);
        }
        else {
            return static_cast<float>(v);
        }
    }
    catch (...) {
        return defaultVal;
    }
}

double clamp01_double(double v) {
    if (v < 0.0) return 0.0;
    if (v > 1.0) return 1.0;
    return v;
}

bool parseSimpleColor(const std::string& s, SimpleColor& out) {
    std::string str = trim_s(s);
    if (str.empty()) return false;
    std::string low = toLowerStr(str);

    int r = 0, g = 0, b = 0;
    // hex #rgb or #rrggbb
    if (parseHexColor_local(low, r, g, b)) {
        out = SimpleColor(static_cast<unsigned char>(r), static_cast<unsigned char>(g), static_cast<unsigned char>(b), 255);
        return true;
    }

    // rgb(r,g,b)
    int rr, gg, bb;
    if (sscanf_s(str.c_str(), "rgb(%d,%d,%d)", &rr, &gg, &bb) == 3) {
        out = SimpleColor(static_cast<unsigned char>(rr), static_cast<unsigned char>(gg), static_cast<unsigned char>(bb), 255);
        return true;
    }

    static const std::unordered_map<std::string, std::tuple<int, int, int>> named = {
        {"black",{0,0,0}}, {"white",{255,255,255}}, {"red",{255,0,0}},
        {"green",{0,128,0}}, {"blue",{0,0,255}}, {"yellow",{255,255,0}},
        {"orange",{255,165,0}}, {"purple",{128,0,128}}, {"gray",{128,128,128}}, {"grey",{128,128,128}}
    };

    auto it = named.find(low);
    if (it != named.end()) {
        auto t = it->second;
        out = SimpleColor((unsigned char)std::get<0>(t), (unsigned char)std::get<1>(t), (unsigned char)std::get<2>(t), 255);
        return true;
    }

    return false;
}

double parseOffsetString(const std::string& s) {
    if (s.empty()) return 0.0;
    std::string t = trim_s(s);
    try {
        if (!t.empty() && t.back() == '%') {
            double v = std::stod(t.substr(0, t.size() - 1));
            return v / 100.0;
        }
        double v = std::stod(t);
        if (v > 1.0) return v / 100.0;
        return v;
    }
    catch (...) {
        return 0.0;
    }
}

// extract id from "url(#id)" (returns empty string if not found)
std::string extractUrlId(const std::string& s) {
    std::regex rx(R"(url\(\s*#([^)\s]+)\s*\))", std::regex::icase);
    std::smatch m;
    if (std::regex_search(s, m, rx) && m.size() >= 2) {
        return m[1].str();
    }
    return "";
}

// convert GDI+ Color -> SimpleColor
SimpleColor colorFromGdiColor(const Gdiplus::Color& c) {
    return SimpleColor(c.GetR(), c.GetG(), c.GetB(), c.GetA());
}

// create and store a Gdiplus::Color in result.colorPool from SimpleColor and return pointer
Gdiplus::Color* makeColorFromSimpleColor(SVGParseResult& result, const SimpleColor& sc) {
    BYTE a = sc.a;
    BYTE r = sc.r;
    BYTE g = sc.g;
    BYTE b = sc.b;
    result.colorPool.push_back(std::make_unique<Gdiplus::Color>(a, r, g, b));
    return result.colorPool.back().get();
}

// resolve a paint string into either a solid color or a gradient reference.
Gdiplus::Color* resolvePaint(SVGParseResult& result, const std::string& paintStr, float opacity, SVGGradient*& outGrad) {
    outGrad = nullptr;
    std::string raw = trim_s(paintStr);
    if (raw.empty()) return nullptr;
    std::string low = toLowerStr(raw);

    // url(#id) detection
    std::string id = extractUrlId(raw);
    if (!id.empty()) {
        auto it = result.gradientsById.find(id);
        if (it != result.gradientsById.end()) {
            outGrad = it->second;
            // produce fallback color (average) with element opacity applied to alpha
            SimpleColor avg = outGrad->averageColor();
            double a = (avg.a / 255.0) * clamp01f(opacity);
            SimpleColor scaled = avg;
            scaled.a = static_cast<unsigned char>(std::round(a * 255.0));
            return makeColorFromSimpleColor(result, scaled);
        }
        return nullptr;
    }

    // not a gradient -> attempt normal color parsing
    return makeColorFromString(result, paintStr, opacity);
}

// parse offset string into 0..1 (handles percent, values >1 as percent if <=100)
double parseOffsetString_local(const std::string& s) {
    if (s.empty()) return 0.0;
    std::string t = trim_s(s);
    if (t.empty()) return 0.0;
    if (t.back() == '%') {
        try {
            double v = std::stod(t.substr(0, t.size() - 1));
            if (v < 0.0) v = 0.0;
            if (v > 100.0) v = 100.0;
            return v / 100.0;
        }
        catch (...) {
            return 0.0;
        }
    }
    try {
        double v = std::stod(t);
        if (v < 0.0) return 0.0;
        if (v > 1.0 && v <= 100.0) {
            return v / 100.0;
        }
        if (v > 1.0) return 1.0;
        return v;
    }
    catch (...) {
        return 0.0;
    }
}

// parse a <stop> node into GradientStop (uses makeColorFromString to parse stop-color)
GradientStop parseStopNode(xml_node<>* stopNode, SVGParseResult& result) {
    GradientStop gs;
    const char* off = nullptr;
    if (auto attr = stopNode->first_attribute("offset")) off = attr->value();
    gs.offset = parseOffsetString_local(off ? off : "0");

    std::string colorStr = getAttrValue(stopNode, "stop-color", "");
    float stopOp = 1.0f;
    if (auto a = stopNode->first_attribute("stop-opacity")) {
        try { stopOp = clamp01f(std::stof(a->value())); }
        catch (...) { stopOp = 1.0f; }
    }

    Gdiplus::Color* gcol = nullptr;
    if (!colorStr.empty()) {
        gcol = makeColorFromString(result, colorStr, stopOp);
    }
    if (!gcol) {
        // fallback black with stopOp alpha
        BYTE alpha = static_cast<BYTE>(clamp01f(stopOp) * 255.0f);
        result.colorPool.push_back(std::make_unique<Gdiplus::Color>(alpha, 0, 0, 0));
        gcol = result.colorPool.back().get();
    }

    gs.color = SimpleColor(gcol->GetR(), gcol->GetG(), gcol->GetB(), gcol->GetA());
    gs.opacity = stopOp;
    return gs;
}

double clamp01(double v) {
    if (v < 0.0) return 0.0;
    if (v > 1.0) return 1.0;
    return v;
}

SimpleColor lerpColor(const SimpleColor& a, const SimpleColor& b, double t) {
    auto lerp = [&](unsigned char A, unsigned char B)->unsigned char {
        double v = A + (B - A) * t;
        if (v < 0) v = 0;
        if (v > 255) v = 255;
        return static_cast<unsigned char>(std::round(v));
        };
    return SimpleColor(lerp(a.r, b.r), lerp(a.g, b.g), lerp(a.b, b.b),
        lerp(a.a, b.a));
}

