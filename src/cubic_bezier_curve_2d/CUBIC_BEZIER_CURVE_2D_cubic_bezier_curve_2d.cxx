#include "cubic_bezier_curve_2d.h"

CubicBezierCurve2d::CubicBezierCurve2d(const std::array<glm::vec2, 4>& ctrl_pts)
	: P(ctrl_pts)
{
}

CubicBezierCurve2d::CubicBezierCurve2d
(
	const glm::vec2& P0,
	const glm::vec2& P1,
	const glm::vec2& P2,
	const glm::vec2& P3
)
	: P{ P0, P1, P2, P3 }
{
}

glm::vec2 CubicBezierCurve2d::Eval(float u) const
{
	return
		P[0] * (-u * u * u     + 3.f * u * u - 3.f * u + 1.f) +
		P[1] * ( 3.f * u * u * u - 6.f * u * u + 3.f * u) +
		P[2] * (-3.f * u * u * u + 3.f * u * u) +
		P[3] *   u * u * u;
}

glm::vec2 CubicBezierCurve2d::EvalFirstDerivative(float u) const
{
	glm::vec2 T1 = (P[1] - P[0]);
	glm::vec2 T2 = (P[2] - P[1]);
	glm::vec2 T3 = (P[3] - P[2]);

	return 3.f * ((1.f - u) * (1.f -u) * T1 + 2.f * u * (1.f - u) * T2 + u * u * T3);
}