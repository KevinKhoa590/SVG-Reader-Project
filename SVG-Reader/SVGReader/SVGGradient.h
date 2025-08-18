#pragma once

#include <string>
#include <vector>
#include <memory>

struct SimpleColor {
    unsigned char a;
    unsigned char r;
    unsigned char g;
    unsigned char b;
    SimpleColor() : a(255), r(0), g(0), b(0) {}
    SimpleColor(unsigned char _r, unsigned char _g, unsigned char _b, unsigned char _a = 255)
        : a(_a), r(_r), g(_g), b(_b) {}
};

struct GradientStop {
    double offset;
    SimpleColor color;
    double opacity;
};

class SVGGradient {
public:
    enum class Units { OBJECT_BOUNDING_BOX, USER_SPACE_ON_USE };

    SVGGradient() : units(Units::OBJECT_BOUNDING_BOX) {}
    virtual ~SVGGradient() = default;

    void addStop(const GradientStop& s) { stops.push_back(s); }
    const std::vector<GradientStop>& getStops() const { return stops; }

    // sample color at t in [0,1] (interpolates between stops)
    virtual SimpleColor sampleAt(double t) const;

    // compute a single fallback color (average) for renderers without gradient support
    SimpleColor averageColor() const;

    Units units;
    std::string id;

    std::string gradientTransform;

    std::string href;

protected:
    std::vector<GradientStop> stops;
};
