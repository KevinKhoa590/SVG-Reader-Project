#include "stdafx.h"
#include <windows.h>
#include <objidl.h>
#include "SVGElement.h"
#include "helper.h"
#include "SVGGradient.h"
#include "SVGLinearGradient.h"
#include "SVGRadialGradient.h"
#include <gdiplus.h>
#include <memory>
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>

#pragma comment(lib, "gdiplus.lib")

using namespace Gdiplus;

std::unique_ptr<Brush> SVGElement::createFillBrush(Graphics& g, const RectF& bounds) const {
    SVGGradient* grad = getFillGradient();
    if (!grad) return nullptr;

    // Linear gradient
    if (auto lg = dynamic_cast<SVGLinearGradient*>(grad)) {
        float defX1 = bounds.X;
        float defY1 = bounds.Y;
        float defX2 = bounds.X + bounds.Width;
        float defY2 = bounds.Y;

        float x1 = parseCoordForLinear(lg->x1, defX1, bounds, lg, true);
        float y1 = parseCoordForLinear(lg->y1, defY1, bounds, lg, false);
        float x2 = parseCoordForLinear(lg->x2, defX2, bounds, lg, true);
        float y2 = parseCoordForLinear(lg->y2, defY2, bounds, lg, false);

        if (std::abs(x2 - x1) < 1e-6f && std::abs(y2 - y1) < 1e-6f) {
            x2 = x1 + 1.0f;
        }

        LinearGradientBrush* brush = new LinearGradientBrush(PointF(x1, y1), PointF(x2, y2), Color::Black, Color::Black);

        const auto& stops = lg->getStops();
        if (!stops.empty()) {
            std::vector<std::pair<double, Color>> items;
            items.reserve(stops.size());
            for (const auto& s : stops) {
                double off = s.offset;
                if (off > 1.0) off = off / 100.0;
                off = clamp01_double(off);

                double srcAlpha = (static_cast<double>(s.color.a) / 255.0);
                double combinedAlpha = srcAlpha * s.opacity;
                if (combinedAlpha < 0.0) combinedAlpha = 0.0;
                if (combinedAlpha > 1.0) combinedAlpha = 1.0;
                BYTE a = static_cast<BYTE>(std::round(combinedAlpha * 255.0));

                Color c(a, s.color.r, s.color.g, s.color.b);
                items.emplace_back(off, c);
            }

            std::sort(items.begin(), items.end(), [](const std::pair<double, Color>& A, const std::pair<double, Color>& B) {
                return A.first < B.first;
            });

            if (items.front().first > 0.0) {
                items.insert(items.begin(), std::make_pair(0.0, items.front().second));
            }
            if (items.back().first < 1.0) {
                items.push_back(std::make_pair(1.0, items.back().second));
            }

            int n = static_cast<int>(items.size());
            std::vector<Color> colors;
            std::vector<REAL> positions;
            colors.reserve(n);
            positions.reserve(n);

            for (int i = 0; i < n; ++i) {
                colors.push_back(items[i].second);
                double pos = items[i].first;
                if (i > 0) {
                    double prev = static_cast<double>(positions[i - 1]);
                    if (pos <= prev) {
                        pos = prev + 1e-6;
                        if (pos > 1.0) pos = 1.0;
                    }
                }
                positions.push_back(static_cast<REAL>(pos));
            }

            if (positions.back() > 1.0f) positions.back() = 1.0f;

            brush->SetInterpolationColors(colors.data(), positions.data(), n);
            brush->SetWrapMode(WrapModeClamp);
        }

        if (!lg->gradientTransform.empty()) {
            std::wstring w = std::wstring(lg->gradientTransform.begin(), lg->gradientTransform.end());
            Gdiplus::Matrix* m = parseTransform(w);
            if (m) {
                brush->MultiplyTransform(m);
                delete m;
            }
        }

        return std::unique_ptr<Brush>(brush);
    }

    // Radial gradient
    if (auto rg = dynamic_cast<SVGRadialGradient*>(grad)) {
        float cx = bounds.X + bounds.Width * 0.5f;
        float cy = bounds.Y + bounds.Height * 0.5f;
        float radius = (bounds.Width > bounds.Height ? bounds.Width : bounds.Height) * 0.5f;
        float fx = cx;
        float fy = cy;

        if (!rg->cx.empty()) cx = parseCoordForRadial(rg->cx, cx, bounds, rg, true);
        if (!rg->cy.empty()) cy = parseCoordForRadial(rg->cy, cy, bounds, rg, false);

        if (!rg->r.empty()) {
            bool usedUserSpace = (rg->units == SVGGradient::Units::USER_SPACE_ON_USE);
            std::string rs = rg->r;
            bool isPercent = (!rs.empty() && rs.back() == '%');
            double rawv = 0.0;
            try { rawv = std::stod(rs); }
            catch (...) { rawv = 1.0; }

            if (usedUserSpace) {
                radius = static_cast<float>(rawv);
            }
            else {
                if (isPercent) {
                    double frac = rawv / 100.0;
                    radius = ((bounds.Width > bounds.Height) ? bounds.Width : bounds.Height) * 0.5f * static_cast<float>(frac);
                }
                else {
                    radius = ((bounds.Width > bounds.Height) ? bounds.Width : bounds.Height) * 0.5f * static_cast<float>(rawv);
                }
            }
        }

        if (!rg->fx.empty()) fx = parseCoordForRadial(rg->fx, fx, bounds, rg, true);
        if (!rg->fy.empty()) fy = parseCoordForRadial(rg->fy, fy, bounds, rg, false);

        if (radius <= 0.0f) radius = 1.0f;

        GraphicsPath path;
        path.AddEllipse(cx - radius, cy - radius, radius * 2.0f, radius * 2.0f);

        PathGradientBrush* brush = new PathGradientBrush(&path);

        const auto& stops = rg->getStops();
        if (!stops.empty()) {
            std::vector<std::pair<double, Color>> items;
            items.reserve(stops.size());
            for (const auto& s : stops) {
                double off = s.offset;
                if (off > 1.0) off = off / 100.0;
                off = clamp01_double(off);

                double srcAlpha = (static_cast<double>(s.color.a) / 255.0);
                double combinedAlpha = srcAlpha * s.opacity;
                if (combinedAlpha < 0.0) combinedAlpha = 0.0;
                if (combinedAlpha > 1.0) combinedAlpha = 1.0;
                BYTE a = static_cast<BYTE>(std::round(combinedAlpha * 255.0));

                items.emplace_back(off, Color(a, s.color.r, s.color.g, s.color.b));
            }

            std::sort(items.begin(), items.end(), [](const std::pair<double, Color>& A, const std::pair<double, Color>& B) {
                return A.first < B.first;
                });

            if (items.front().first > 0.0) items.insert(items.begin(), std::make_pair(0.0, items.front().second));
            if (items.back().first < 1.0) items.push_back(std::make_pair(1.0, items.back().second));

            Color centerCol = items.front().second;
            brush->SetCenterColor(centerCol);

            Color surroundCol = items.back().second;
            INT surroundCount = 1;
            brush->SetSurroundColors(&surroundCol, &surroundCount);

            brush->SetCenterPoint(PointF(fx, fy));
            brush->SetWrapMode(WrapModeClamp);
        }
        else {
            SimpleColor avg = rg->averageColor();
            Color c = simpleToGdi(avg);
            delete brush;
            return std::make_unique<SolidBrush>(c);
        }

        if (!rg->gradientTransform.empty()) {
            std::wstring w = std::wstring(rg->gradientTransform.begin(), rg->gradientTransform.end());
            Gdiplus::Matrix* m = parseTransform(w);
            if (m) {
                brush->MultiplyTransform(m);
                delete m;
            }
        }

        return std::unique_ptr<Brush>(brush);
    }

    SimpleColor avg = grad->averageColor();
    return std::make_unique<SolidBrush>(simpleToGdi(avg));
}

std::unique_ptr<Brush> SVGElement::createStrokeBrush(Graphics& g, const RectF& bounds) const {
    SVGGradient* grad = getStrokeGradient();
    if (!grad) return nullptr;
    return createFillBrush(g, bounds);
}

void SVGElement::elementRender(Gdiplus::Graphics& g) {
    GraphicsState state = g.Save();
    g.MultiplyTransform(transform.getMatrix());
    draw(g);
    g.Restore(state);
}
