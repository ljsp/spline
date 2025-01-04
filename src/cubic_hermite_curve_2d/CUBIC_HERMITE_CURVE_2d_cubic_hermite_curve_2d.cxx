#include "cubic_hermite_curve_2d.h"

CubicHermiteCurve2d::CubicHermiteCurve2d
(
	const glm::vec2& P0_,
	const glm::vec2& P1_,
	const glm::vec2& P2_,
	const glm::vec2& P3_
)
	: P0(P0_), P1(P3_), N0(P1_ - P0_), N1(P2_ - P3_)
{
}

glm::vec2 CubicHermiteCurve2d::Eval(float u) const
{
	return 
		P0 * ( 2.f * u * u * u - 3.f * u * u + 1.f) +
		N0 * ( 3.f * u * u * u - 6.f * u * u + 3.f * u) +
		P1 * (-2.f * u * u * u + 3.f * u * u) +
		N1 * (-3.f * u * u * u + 3.f * u * u);
}

glm::vec2 CubicHermiteCurve2d::EvalFirstDerivative(float u) const
{
	float h00_prime =  6.f * u * u - 6.f * u;
	float h10_prime =  9.f * u * u - 12.f * u + 3.f;
	float h01_prime = -6.f * u * u + 6.f * u;
	float h11_prime = -9.f * u * u + 6.f * u;

	return h00_prime * P0 + h10_prime * N0 + h01_prime * P1 + h11_prime * N1;
}
glm::vec2 CubicHermiteCurve2d::EvalSecondDerivative(float u) const
{
	float h00_prime_prime =  12.f * u - 6.f;
	float h10_prime_prime =  18.f * u - 12.f;
	float h01_prime_prime = -12.f * u + 6.f;
	float h11_prime_prime = -18.f * u + 6.f;

	return h00_prime_prime * P0 + h10_prime_prime * N0 + h01_prime_prime * P1 + h11_prime_prime * N1;
}