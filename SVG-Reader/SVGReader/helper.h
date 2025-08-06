#pragma once

#include "rapidxml.hpp"
#include <string>
#include <vector>

using namespace rapidxml;
namespace Gdiplus {
	class Color;
	class Point;
	class Matrix;
}

Gdiplus::Color parseColor(xml_attribute<>* attr);
float parseOpacity(xml_attribute<>* attr);
std::vector<Gdiplus::Point> parsePoints(const std::string& pointStr);
Gdiplus::Matrix* parseTransform(const std::wstring& transformStr);
