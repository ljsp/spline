 #include <cassert>

#include "../cubic_hermite_spline_2d/cubic_hermite_spline_2d.h"
#include "../cubic_bezier_spline_2d/cubic_bezier_spline_2d.h"

const double epsilon = std::numeric_limits<double>::epsilon();

CubicHermiteSpline2d::CubicHermiteSpline2d(const std::vector< glm::vec2 >& ctrl_pts)
{
    assert(ctrl_pts.size() % 4 == 0);

    for (size_t i = 0; i < ctrl_pts.size(); i += 4)
    {
        m_curves.emplace_back(ctrl_pts[i], ctrl_pts[i + 1], ctrl_pts[i + 2], ctrl_pts[i + 3]);
    }
}

CubicHermiteSpline2d::CubicHermiteSpline2d
(
    const std::vector< glm::vec2 >& ctrl_pts,
    const std::vector< glm::vec2 >& tangent_vectors
)
{
     assert(ctrl_pts.size() % 2 == 0);
     assert(ctrl_pts.size() == tangent_vectors.size());

    for (size_t i = 0; i < ctrl_pts.size(); i += 2)
    {
        m_curves.emplace_back(ctrl_pts[i], ctrl_pts[i + 1], tangent_vectors[i], tangent_vectors[i + 1]);
    }
}

CubicHermiteSpline2d CubicHermiteSpline2d::FromCubicBezierSpline2d(const CubicBezierSpline2d& cubic_bezier_spline_2d)
{
    std::vector<glm::vec2> ctrl_pts(cubic_bezier_spline_2d.m_curves.size() * 4);
    size_t index = 0;
    for (const CubicBezierCurve2d& cubic_bezier_curve_2d : cubic_bezier_spline_2d.m_curves)
    {
        for (const glm::vec2& ctrl_pt : cubic_bezier_curve_2d.P)
        {
            ctrl_pts[index] = ctrl_pt;
            ++index;
        }
    }
    return CubicHermiteSpline2d(ctrl_pts);
}

void CubicHermiteSpline2d::AddCurve(const CubicHermiteCurve2d& cubic_hermite_curve_2d)
{
    m_curves.push_back(cubic_hermite_curve_2d);
}

void CubicHermiteSpline2d::RemoveCurve(size_t index)
{
    m_curves.erase(m_curves.begin() + index);
}

bool CubicHermiteSpline2d::IsC1Continuous() const
{
    if (m_curves.size() < 2)
    {
        return true;
    }

    bool is_c1_continuous = true;
    size_t i = 0;
    while (is_c1_continuous && i < m_curves.size() - 1)
    {
        if (m_curves[i].EvalFirstDerivative(1.0) != m_curves[i + 1].EvalFirstDerivative(0.))
        {
            is_c1_continuous = false;
        }
        ++i;
    }
    return is_c1_continuous;
}

bool CubicHermiteSpline2d::IsC2Continuous() const
{
    if (m_curves.size() < 2)
    {
        return true;
    }

    bool is_c2_continuous = true;
    size_t i = 0;
    while (is_c2_continuous && i < m_curves.size() - 1)
    {
        if (m_curves[i].EvalSecondDerivative(1.0) != m_curves[i + 1].EvalSecondDerivative(0.))
        {
            is_c2_continuous = false;
        }
        ++i;
    }
    return is_c2_continuous;
}

glm::vec2 CubicHermiteSpline2d::Eval(double t) const
{
    if (t <= 0.0)
    {
        return m_curves.front().P0;
    }
    else if (1.0 <= t + epsilon)
    {
        return m_curves.back().P1;
    }

    t = std::max(t, 0.0);
    t = std::min(t, 1.0);
    t *= static_cast<double>(m_curves.size());
    double t_decimal = t - static_cast<int>(t);
    int t_integer = t - t_decimal;

    return m_curves[t_integer].Eval(t_decimal);
}

glm::vec2 CubicHermiteSpline2d::EvalFirstDerivative(double t) const
{
    t = std::max(t, 0.);
    t = std::min(t, 1. - epsilon);
    t *= static_cast<double>(m_curves.size());
    double t_decimal = t - static_cast<int>(t);
    int t_integer = t - t_decimal;

    return m_curves[t_integer].EvalFirstDerivative(t_decimal);
}

glm::vec2 CubicHermiteSpline2d::EvalSecondDerivative(double t) const
{
    t = std::max(t, 0.);
    t = std::min(t, 1. - epsilon);
    t *= static_cast<double>(m_curves.size());
    double t_decimal = t - static_cast<int>(t);
    int t_integer = t - t_decimal;

    return m_curves[t_integer].EvalSecondDerivative(t_decimal);
}