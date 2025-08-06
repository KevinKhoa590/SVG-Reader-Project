#include "stdafx.h"
#include "transform.h"
#include <windows.h>
#include <objidl.h>
#include <gdiplus.h>
#include <sstream>
#include <regex>
#include <cmath>

#pragma comment(lib, "gdiplus.lib")
using namespace Gdiplus;
using namespace std;

Transform::Transform() {
    matrix = new Matrix();
}

Transform::~Transform() {
    delete matrix;
}

void Transform::applyTransform(const wstring& transformStr) {
    wregex regex(L"(\\w+)\\s*\\(([^\\)]+)\\)");
    auto begin = wsregex_iterator(transformStr.begin(), transformStr.end(), regex);
    auto end = wsregex_iterator();

    for (auto it = begin; it != end; ++it) {
        wstring type = it->str(1);
        wstring args = it->str(2);
        wistringstream argStream(args);
        vector<float> values;
        float val;

        while (argStream >> val) {
            values.push_back(val);
            if (argStream.peek() == ',' || argStream.peek() == ' ')
                argStream.ignore();
        }

        if (type == L"translate") {
            float dx = values.size() > 0 ? values[0] : 0;
            float dy = values.size() > 1 ? values[1] : 0;
            Matrix temp;
            temp.Translate(dx, dy);
            matrix->Multiply(&temp, MatrixOrderAppend);
        }
        else if (type == L"scale") {
            float sx = values.size() > 0 ? values[0] : 1;
            float sy = values.size() > 1 ? values[1] : sx;
            Matrix temp;
            temp.Scale(sx, sy);
            matrix->Multiply(&temp, MatrixOrderAppend);
        }
        else if (type == L"rotate") {
            float angle = values.size() > 0 ? values[0] : 0;
            Matrix temp;
            if (values.size() == 3) {
                temp.RotateAt(angle, PointF(values[1], values[2]));
            }
            else {
                temp.Rotate(angle);
            }
            matrix->Multiply(&temp, MatrixOrderAppend);
        }
        else if (type == L"matrix" && values.size() == 6) {
            Matrix temp(values[0], values[1], values[2], values[3], values[4], values[5]);
            matrix->Multiply(&temp, MatrixOrderAppend);
        }
    }
}

const Matrix* Transform::getMatrix() const {
    return matrix;
}
