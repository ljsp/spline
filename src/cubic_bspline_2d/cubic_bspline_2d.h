#pragma once

#include <glm/glm.hpp>
#include <vector>

#include "../cubic_bezier_spline_2d/cubic_bezier_spline_2d.h"
#include "../cubic_hermite_spline_2d/cubic_hermite_spline_2d.h"

// =============================================================================
class CubicBSpline2d
{
public:
    explicit CubicBSpline2d(const std::vector< glm::vec2 >& ctrl_pts);
    
    std::vector<double> ComputeKnots(size_t nb_ctrl_pts) const;

    glm::vec2 Eval(double t) const;
    glm::vec2 EvalFirstDerivative(double t) const;
    glm::vec2 EvalSecondDerivative(double t) const;

    std::vector< glm::vec2 > m_ctrl_pts;
    std::vector< double >    m_knots;
};