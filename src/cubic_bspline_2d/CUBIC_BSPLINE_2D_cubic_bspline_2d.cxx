#include "../cubic_bezier_spline_2d/cubic_bezier_spline_2d.h"
#include "../cubic_bspline_2d/cubic_bspline_2d.h"
#include "../cubic_hermite_spline_2d/cubic_hermite_spline_2d.h"

namespace
{
    const double epsilon = std::numeric_limits<double>::epsilon();

    float N(double u, int i, int deg, const std::vector< double >& knots)
    {
        double ret = 0.0;
        if (deg == 0)
        {
            if (knots[i] <= u && u < knots[i + 1])
            {
                ret = 1.0;
            }
            else
            {
                ret = 0.0;
            }
        }
        else
        {

            double N0 = 0.0;
            double Q0 = (u - knots[i]);
            double D0 = (knots[i + deg] - knots[i]);

            if (D0 < epsilon)
            {
                Q0 = 0.0;
                D0 = 1.0;
            }
            else
            {
                N0 = N(u, i, deg - 1, knots);
            }

            double N1 = 0.0;//N( u, i+1, deg-1, knots );
            double Q1 = (knots[i + deg + 1] - u);
            double D1 = (knots[i + deg + 1] - knots[i + 1]);

            if (D1 < epsilon)
            {
                Q1 = 0.0;
                D1 = 1.0;
            }
            else
            {
                N1 = N(u, i + 1, deg - 1, knots);
            }


            ret = N0 * Q0 / D0 + N1 * Q1 / D1;

        }
        return static_cast<float>(ret);
    }
}

CubicBSpline2d::CubicBSpline2d(const std::vector<glm::vec2>& ctrl_pts)
    : m_ctrl_pts(ctrl_pts)
{
    m_knots = ComputeKnots(ctrl_pts.size());
}

std::vector<double> CubicBSpline2d::ComputeKnots(size_t nb_ctrl_pts) const
{
    assert(nb_ctrl_pts > 3);
    
    size_t knots_values_size = nb_ctrl_pts + 4;
    std::vector<double> knots_values(knots_values_size);
    
    for (int i = 0; i < 3; i++)
    {
        knots_values[i] = 0.;
		knots_values[knots_values_size - 1 - i] = 1.;
    }

    auto denominator = static_cast<double>(knots_values_size - 7);
    for (int i = 4; i < knots_values.size() - 3; i++)
    {
        knots_values[i] = (i-3) / denominator;
    }

    return knots_values;
}

glm::vec2 CubicBSpline2d::Eval
(
    double t
) const
{
    if (t <= 0.0)
    {
        return m_ctrl_pts.front();
    }
    else if (1.0 <= t + epsilon)
    {
        return m_ctrl_pts.back();
    }

    t = std::max(t, 0.0);
    t = std::min(t, 1.0);

    glm::vec2 eval_pt{};
    size_t it_size = m_ctrl_pts.size();
    for (size_t it = 0; it < it_size; ++it)
    {
        eval_pt += N(t, (int)it, 3, m_knots) * m_ctrl_pts[it];
    }
    return eval_pt;
}

glm::vec2 CubicBSpline2d::EvalFirstDerivative
(
    double t
) const
{
    if (t <= 0.0)
    {
        t = 0.0;
    }
    else if (1.0 <= t + epsilon)
    {
        t = 1.0;
    }

    t = std::max(t, 0.0);
    t = std::min(t, 1.0);

    glm::vec2 eval_pt{};
    size_t it_size = m_ctrl_pts.size();
    for (size_t it = 0; it < it_size - 2; ++it)
    {
        eval_pt += N(t, (int)it + 1, 2, m_knots) * (m_ctrl_pts[it + 1] - m_ctrl_pts[it]);
    }
    return eval_pt;
}

glm::vec2 CubicBSpline2d::EvalSecondDerivative
(
    double t
) const
{
    if (t <= 0.0)
    {
        t = 0.0;
    }
    else if (1.0 <= t + epsilon)
    {
        t = 1.0;
    }

    t = std::max(t, 0.0);
    t = std::min(t, 1.0);

    glm::vec2 eval_pt{};
    size_t it_size = m_ctrl_pts.size();
    for (size_t it = 0; it < it_size - 3; ++it)
    {
        eval_pt += N(t, (int)it + 2, 1, m_knots) * (m_ctrl_pts[it + 2] - m_ctrl_pts[it]);
    }
    return eval_pt;
}