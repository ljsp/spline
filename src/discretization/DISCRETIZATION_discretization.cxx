#include "../discretization/discretization.h"

std::vector<glm::vec2> Discretization::Linear
(
    CubicBezierCurve2d const& cubicBezierCurve2d, 
    uint32_t nb_pts
)
{
    std::vector<glm::vec2> polylines;

    assert(nb_pts >= 2);
    double t_step = 1.0 / ((int32_t)nb_pts - 1);
    for (uint32_t i = 0; i < nb_pts; ++i)
    {
        double t = std::min(i * t_step, 1.0);
        glm::vec2 pt = cubicBezierCurve2d.Eval(t);
        polylines.push_back(pt);
    }

    return polylines;
}

std::vector<glm::vec2> Discretization::Linear
(
    CubicBezierSpline2d const& cubicBezierSpline2d,
    uint32_t nb_pts
)
{
    std::vector<glm::vec2> polylines;

    assert(nb_pts >= 2);
    double t_step = 1.0 / ((int32_t)nb_pts - 1);
    for (uint32_t i = 0; i < nb_pts; ++i)
    {
        double t = std::min(i * t_step, 1.0);
        glm::vec2 pt = cubicBezierSpline2d.Eval(t);
        polylines.push_back(pt);
    }

    return polylines;
}

std::vector<glm::vec2> Discretization::Linear
(
    CubicHermiteCurve2d const& cubicHermiteCurve2d,
    uint32_t nb_pts
)
{
    std::vector<glm::vec2> polylines;

    assert(nb_pts >= 2);
    double t_step = 1.0 / ((int32_t)nb_pts - 1);
    for (uint32_t i = 0; i < nb_pts; ++i)
    {
        double t = std::min(i * t_step, 1.0);
        glm::vec2 pt = cubicHermiteCurve2d.Eval(t);
        polylines.push_back(pt);
    }

    return polylines;
}

std::vector<glm::vec2> Discretization::Linear
(
    CubicHermiteSpline2d const& cubicHermiteSpline2d,
    uint32_t nb_pts
)
{
    std::vector<glm::vec2> polylines;

    assert(nb_pts >= 2);
    double t_step = 1.0 / ((int32_t)nb_pts - 1);
    for (uint32_t i = 0; i < nb_pts; ++i)
    {
        double t = std::min(i * t_step, 1.0);
        glm::vec2 pt = cubicHermiteSpline2d.Eval(t);
        polylines.push_back(pt);
    }

    return polylines;
}

std::vector<glm::vec2> Discretization::Linear
(
    CubicBSpline2d const& cubicBSpline2d,
    uint32_t nb_pts
)
{
    std::vector<glm::vec2> polylines;

    assert(nb_pts >= 2);
    double t_step = 1.0 / ((int32_t)nb_pts - 1);
    for (uint32_t i = 0; i < nb_pts; ++i)
    {
        double t = std::min( i * t_step, 1.0 );
        glm::vec2 pt = cubicBSpline2d.Eval(t);
        polylines.push_back(pt);
    }

    return polylines;
}