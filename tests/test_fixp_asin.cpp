#include "fixp.h"

#include <assert.h>
#include <cmath>

static void check(fixp result, double expected, double tol)
{
	double r = result.todouble();
	assert(r > expected - tol && r < expected + tol);
}

int main()
{
	const double tol = 0.0002;

	check(fixp_asin(fixp(0.0)),   0.0,          tol);
	check(fixp_asin(fixp(1.0)),   M_PI / 2.0,   tol);
	check(fixp_asin(fixp(-1.0)), -M_PI / 2.0,   tol);
	check(fixp_asin(fixp(0.5)),   asin(0.5),     tol);
	check(fixp_asin(fixp(-0.5)),  asin(-0.5),    tol);
	check(fixp_asin(fixp(0.25)),  asin(0.25),    tol);
	check(fixp_asin(fixp(-0.25)), asin(-0.25),   tol);
	check(fixp_asin(fixp(0.75)),  asin(0.75),    tol);
	check(fixp_asin(fixp(-0.75)), asin(-0.75),   tol);
	check(fixp_asin(fixp(0.9)),   asin(0.9),     tol);
	check(fixp_asin(fixp(-0.9)),  asin(-0.9),    tol);

	return 0;
}
