#pragma once
#include <string>
#include <vector>

namespace Gdiplus {
    class Matrix;
}

class Transform {
private:
    Gdiplus::Matrix* matrix;

public:
    Transform();
    ~Transform();
    void applyTransform(const std::wstring& transformStr);
    const Gdiplus::Matrix* getMatrix() const;
};

