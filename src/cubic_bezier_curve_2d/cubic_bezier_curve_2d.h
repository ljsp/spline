#pragma once

#include "glm/vec2.hpp"
#include <array>

class CubicBezierCurve2d
{
public:
    explicit CubicBezierCurve2d(const std::array< glm::vec2, 4 >& ctrl_pts);
    CubicBezierCurve2d(const glm::vec2& P0, const glm::vec2& P1, const glm::vec2& P2, const glm::vec2& P3);

    glm::vec2 Eval(float u) const;
    glm::vec2 EvalFirstDerivative(float t) const;

    std::array< glm::vec2, 4 > P;
};