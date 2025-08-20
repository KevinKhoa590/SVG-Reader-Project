#pragma once
#include "stdafx.h"
#include "rapidxml.hpp"
#include "SVGParser.h"
#include "SVGGroup.h"
#include "SVGGradient.h"
#include "SVGLinearGradient.h"
#include "SVGRadialGradient.h"
#include <windows.h>
#include <objidl.h>
#include <gdiplus.h>
#include <string>
#include <vector>

using namespace Gdiplus;
using namespace rapidxml;

Gdiplus::Color parseColor(xml_attribute<>* attr);
float parseOpacity(xml_attribute<>* attr);
std::vector<Gdiplus::Point> parsePoints(const std::string& pointStr);
Gdiplus::Matrix* parseTransform(const std::wstring& transformStr);

xml_attribute<>* getAttr(xml_node<>* node, const char* name);
std::string getAttrValue(xml_node<>* node, const char* name, const std::string& def = "");
int parseIntAttr(xml_node<>* node, const char* name, int def);
void applyAttributesIfPresent(SVGElement* element, xml_node<>* node);
float clamp01(float v);

std::string trim_s(const std::string& s);
float clamp01f(float v);
int hexDigit(char c);
bool parseHexColor_local(const std::string& s, int& r, int& g, int& b);
std::string toLowerStr(const std::string& s);
std::string narrow(const std::wstring& ws);
float parseOpacityFromWString(const std::wstring& w, float def = 1.0f);
Gdiplus::Color* makeColorFromString(SVGParseResult& result, const std::string& rawIn, float opacity = 1.0f);
std::string effectiveAttrString(xml_node<>* node, const char* attrName, SVGGroup* parentGroup);
float effectiveOpacity(xml_node<>* node, const char* attrName, SVGGroup* parentGroup, float def = 1.0f);
int effectiveStrokeWidth(xml_node<>* node, SVGGroup* parentGroup, int def = 1);

Gdiplus::Color simpleToGdi(const SimpleColor& sc);
float parseCoordForLinear(const std::string& s, float defaultVal, const RectF& bounds, const SVGLinearGradient* lg, bool isX);
float parseCoordForRadial(const std::string& s, float defaultVal, const RectF& bounds, const SVGGradient* grad, bool isX);
double clamp01_double(double v);

bool parseSimpleColor(const std::string& s, SimpleColor& out);
double parseOffsetString(const std::string& s);

std::string extractUrlId(const std::string& s);
SimpleColor colorFromGdiColor(const Gdiplus::Color& c);
Gdiplus::Color* makeColorFromSimpleColor(SVGParseResult& result, const SimpleColor& sc);
Gdiplus::Color* resolvePaint(SVGParseResult& result, const std::string& paintStr, float opacity, SVGGradient*& outGrad);
GradientStop parseStopNode(xml_node<>* stopNode, SVGParseResult& result);
double parseOffsetString_local(const std::string& s);

double clamp01(double v);
SimpleColor lerpColor(const SimpleColor& a, const SimpleColor& b, double t);



