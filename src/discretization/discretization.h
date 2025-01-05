#pragma once

#include "../cubic_bezier_spline_2d/cubic_bezier_spline_2d.h"
#include "../cubic_hermite_spline_2d/cubic_hermite_spline_2d.h"
#include "../cubic_bspline_2d/cubic_bspline_2d.h"

namespace Discretization
{
    std::vector<glm::vec2> Linear(  CubicBSpline2d          const& cubicBSpline2d,          uint32_t nb_pts );
    std::vector<glm::vec2> Linear(  CubicBezierCurve2d      const& cubicBezierCurve2d,      uint32_t nb_pts );
    std::vector<glm::vec2> Linear(  CubicBezierSpline2d     const& cubicBezierSpline2d,     uint32_t nb_pts );
    std::vector<glm::vec2> Linear(  CubicHermiteCurve2d     const& cubicHermiteCurve2d,     uint32_t nb_pts );
    std::vector<glm::vec2> Linear(  CubicHermiteSpline2d    const& cubicHermiteSpline2d,    uint32_t nb_pts );
};
