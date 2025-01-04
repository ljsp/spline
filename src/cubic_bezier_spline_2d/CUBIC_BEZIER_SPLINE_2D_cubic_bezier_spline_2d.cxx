#include "../cubic_bezier_spline_2d/cubic_bezier_spline_2d.h"
#include "../cubic_hermite_spline_2d/cubic_hermite_spline_2d.h"

#include <limits>

const double epsilon = std::numeric_limits<double>::epsilon();

CubicBezierSpline2d::CubicBezierSpline2d(const std::vector< glm::vec2 >& ctrl_pts)
{
    for (size_t i = 0; i < ctrl_pts.size() - 3; i += 4)
    {
        m_curves.emplace_back(ctrl_pts[i], ctrl_pts[i + 1], ctrl_pts[i + 2], ctrl_pts[i + 3]);
    }
}

CubicBezierSpline2d CubicBezierSpline2d::FromCubicHermiteSpline2d(const CubicHermiteSpline2d& cubic_hermite_spline_2d)
{
    size_t nb_ctrl_pts = cubic_hermite_spline_2d.m_curves.size() * 4;
    std::vector<glm::vec2> ctrl_pts(nb_ctrl_pts);

    for (size_t i = 0; i < cubic_hermite_spline_2d.m_curves.size(); i++)
    {
        ctrl_pts[i * 4]     = cubic_hermite_spline_2d.m_curves[i].P0;
        ctrl_pts[i * 4 + 1] = cubic_hermite_spline_2d.m_curves[i].P0 + cubic_hermite_spline_2d.m_curves[i].N0;
        ctrl_pts[i * 4 + 2] = cubic_hermite_spline_2d.m_curves[i].P1 + cubic_hermite_spline_2d.m_curves[i].N1;
        ctrl_pts[i * 4 + 3] = cubic_hermite_spline_2d.m_curves[i].P1;
    }

    return CubicBezierSpline2d(ctrl_pts);
}

CubicBezierSpline2d CubicBezierSpline2d::FromCubicBSplinePoints2d(const std::vector<glm::vec2>& ctrl_pts)
{
    std::vector<glm::vec2> resized_ctrl_pts = ctrl_pts;
    for (size_t i = 0; i < ctrl_pts.size() % 3; i++)
    {
        resized_ctrl_pts.pop_back();
    }

    std::vector<glm::vec2> bezier_ctrl_pts;
    for (size_t i = 0; i < resized_ctrl_pts.size() - 2; i+= 3)
    {
        const glm::vec2& P0 = resized_ctrl_pts[i];
        const glm::vec2& P1 = resized_ctrl_pts[i + 1];
        const glm::vec2& P2 = resized_ctrl_pts[i + 2];
        const glm::vec2& P3 = resized_ctrl_pts[i + 3];

        bezier_ctrl_pts.push_back(P0);
        bezier_ctrl_pts.push_back(P1);
        bezier_ctrl_pts.push_back(P2);
        bezier_ctrl_pts.push_back(P3);
    }
    
    return CubicBezierSpline2d(bezier_ctrl_pts);
}

void CubicBezierSpline2d::AddCurve(const CubicBezierCurve2d& cubic_bezier_curve_2d)
{
    m_curves.push_back(cubic_bezier_curve_2d);
}

void CubicBezierSpline2d::RemoveCurve(size_t index)
{
    m_curves.erase(m_curves.begin() + index);
}

bool CubicBezierSpline2d::IsC1Continuous() const
{
    if (m_curves.size() < 2)
    {
        return true;
    }
    bool is_c1_continuous = true;
    size_t i = 0;
    while(is_c1_continuous && i < m_curves.size() - 1)
    {
        if (m_curves[i].EvalFirstDerivative(1.0) != m_curves[i + 1].EvalFirstDerivative(0.))
        {
            is_c1_continuous = false;
        }
        ++i;
    }
    return is_c1_continuous;
}

glm::vec2 CubicBezierSpline2d::Eval(double t) const
{
    if (t <= 0.0)
    {
        return m_curves.front().P[0];
    }
    else if (t >= 1.0 - epsilon)
    {
        return m_curves.back().P[3];
    }

    t = std::max(t, 0.0);
    t = std::min(t, 1.0);
    t *= static_cast<double>(m_curves.size());
    double t_decimal = t - static_cast<int>(t);
    int t_integer = t - t_decimal;

    return m_curves[t_integer].Eval(t_decimal);
}

glm::vec2 CubicBezierSpline2d::EvalFirstDerivative(double t) const
{
    t = std::max(t, 0.);
    t = std::min(t, 1. - epsilon);
    t *= static_cast<double>(m_curves.size());
    double t_decimal = t - static_cast<int>(t);
    int t_integer = t - t_decimal;

    return m_curves[t_integer].EvalFirstDerivative(t_decimal);
}

std::vector<glm::vec2> CubicBezierSpline2d::GetControlPoints() const
{
    std::vector<glm::vec2> ctrl_pts(m_curves.size() * 4);
    for (size_t i = 0; i < m_curves.size(); i++)
    {
        ctrl_pts[i * 4]     = m_curves[i].P[0];
        ctrl_pts[i * 4 + 1] = m_curves[i].P[1];
        ctrl_pts[i * 4 + 2] = m_curves[i].P[2];
        ctrl_pts[i * 4 + 3] = m_curves[i].P[3];
    }
    return ctrl_pts;
}
