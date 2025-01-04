#pragma once

#include <vector>

#include "../cubic_bezier_curve_2d/cubic_bezier_curve_2d.h"
#include "../cubic_hermite_spline_2d/cubic_hermite_spline_2d.h"

class CubicHermiteSpline2d;

class CubicBezierSpline2d
{
public:
    explicit CubicBezierSpline2d(const std::vector< glm::vec2 >& ctrl_pts);

    static CubicBezierSpline2d FromCubicHermiteSpline2d(const CubicHermiteSpline2d& cubic_hermite_spline_2d);
    //static CubicBezierSpline2d FromCubicBSpline2d(const CubicBSpline2d& cubic_bspline_2d);
    static CubicBezierSpline2d FromCubicBSplinePoints2d(const std::vector <glm::vec2>& ctrl_pts);

    void AddCurve(const CubicBezierCurve2d& cubic_bezier_curve_2d);
    void RemoveCurve(size_t index);

    bool IsC1Continuous() const;

    glm::vec2 Eval(double t) const;
    glm::vec2 EvalFirstDerivative(double t) const;

    std::vector<glm::vec2> GetControlPoints() const;

    std::vector< CubicBezierCurve2d > m_curves;   
};