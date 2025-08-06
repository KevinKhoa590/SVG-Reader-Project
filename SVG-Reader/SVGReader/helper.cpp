#include "stdafx.h"
#include "rapidxml.hpp"
#include "helper.h"
#include <windows.h>
#include <objidl.h>
#include <gdiplus.h>
#include <regex>
#include <sstream>
#include <map>
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

