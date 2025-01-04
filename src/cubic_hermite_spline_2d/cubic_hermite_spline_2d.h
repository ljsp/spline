#pragma once

#include <glm/glm.hpp>
#include <vector>

#include "../cubic_hermite_curve_2d/cubic_hermite_curve_2d.h"
#include "../cubic_bezier_spline_2d/cubic_bezier_spline_2d.h"

class CubicBezierSpline2d;

class CubicHermiteSpline2d
{
public:
    explicit CubicHermiteSpline2d(const std::vector< glm::vec2 >& ctrl_pts);
    CubicHermiteSpline2d(const std::vector< glm::vec2 >& ctrl_pts, const std::vector< glm::vec2 >& tangent_vectors);

    static CubicHermiteSpline2d FromCubicBezierSpline2d(const CubicBezierSpline2d& cubic_bezier_spline_2d);
    //static CubicHermiteSpline2d FromCubicBSpline2d(const CubicBSpline2d& cubic_bspline_2d);

    glm::vec2 Eval(double t) const;
    glm::vec2 EvalFirstDerivative(double t) const;
    glm::vec2 EvalSecondDerivative(double t) const;

    void AddCurve(const CubicHermiteCurve2d& cubic_hermite_curve_2d);
    void RemoveCurve(size_t index);

    bool IsC1Continuous() const;
    bool IsC2Continuous() const;

    std::vector< CubicHermiteCurve2d > m_curves;
};