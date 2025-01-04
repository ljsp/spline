#pragma once

#include <glm/glm.hpp>
#include <array>

class CubicHermiteCurve2d
{
public:
    //CubicHermiteCurve2d(const glm::vec2& P0, const glm::vec2& P1, const glm::vec2& N0, const glm::vec2& N1);
    CubicHermiteCurve2d(const glm::vec2& P0, const glm::vec2& P1, const glm::vec2& P2, const glm::vec2& P3);
    
    glm::vec2 Eval(float u) const;
    glm::vec2 EvalFirstDerivative(float t) const;
    glm::vec2 EvalSecondDerivative(float t) const;

    glm::vec2 P0;
    glm::vec2 P1;
    glm::vec2 N0;
    glm::vec2 N1;
};