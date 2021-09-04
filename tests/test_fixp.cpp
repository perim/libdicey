#include "fixp.h"

#include <assert.h>
#include <stdio.h>
#include <cstdint>
#include <cmath>

static inline double dotf(ivec2 a, ivec2 b) { return a.x.todouble() * b.x.todouble() + a.y.todouble() * b.y.todouble(); }

int main()
{
	fixp f1 = 1;
	fixp f2 = 1.5;
	fixp f3 = 1.75f;
	fixp fdiv = f1 / f3;
	assert(fdiv == 0.571428571);
	fixp f4;
	assert(f1 != f2 && f2 != f3 && f1 != f3 && f1 != f4);
	assert(f1 == 1 && f2 == 1.5 && f3 == 1.75f && f4 == 0 && f4 == 0.0f);
	assert(f2 > f1);
	assert(f1 < f2);
	assert(f2 >= f1);
	assert(f1 <= f2);
	assert(f1 == f1);
	assert(f1 != f2);

	fixp f314 = 3.14f;
	assert(f314.tofloat() > 3.13f);
	assert(f314.tofloat() < 3.15f);

	fixp f10 = 10;
	assert(f10 / f1 == f10);
	assert(f10 / f1 == 10);
	fixp f5 = 2;
	assert(f10 / f5 == 5);
	assert(f1 / f1 == 1);

	fixp fl1 = 40.000000;
	fixp fl2 = 56.568542;
	fixp fll = fl1 / fl2;
	fixp fres = 0.707107;
	assert(fll >= fres - fres * 0.1 && fll <= fres + fres * 0.1);

	assert(f1.tofloat() == 1.0f);
	assert(f1.todouble() == 1.0);
	assert(f1.toint() == 1);
	assert(f2.toint() == 1); // round down
	assert(f3.toint() == 1); // always
	assert(f4.toint() == 0);
	assert(f1 + f4 == 1);
	assert(f1 + 1 == 2);
	assert(f1 - 1 == 0);
	assert(f4 - 0 == 0);
	assert(f1 * 4 == 4);

	fixp fn1 = -2;
	fixp fn2 = -16;
	assert(fn2 - fn1 == -14);
	assert(fn2 + fn1 == -18);
	assert(fn2 * fn1 == 32);
	assert(fn2 / fn1 == 8);

	aabb a;
	a.min = { 0, 0 };
	a.max = { 40, 40 };
	aabb b { { 30, 30 }, { 45, 45 } };
	aabb c = { { 330, 330 }, { 345, 345 } };

	assert(fixp_overlaps(a, b));
	assert(!fixp_overlaps(a, c));

	assert(fixp_intersect(f10, f5) == 10.0/(10.0-2.0));
	assert(fixp_dot(a.min, a.max) == dotf(a.min, a.max)); //a.min.x * a.max.x + a.min.y * a.max.y);
	assert(fixp_pow(f5, 1) == 2);
	assert(fixp_pow(f5, 2) == 4);
	assert(fixp_sqrt(f5).toint() == 1);
	assert(fixp_sqrt(f5).tofloat() > 1.4f);
	assert(fixp_sqrt(f5).tofloat() < 1.5f);
	assert(fixp_sqrt(f10).tofloat() > 3.1f && fixp_sqrt(f10).tofloat() < 3.2f);
	double distf = sqrt(pow((a.min.x.todouble() - b.max.x.todouble()), 2) + pow((a.min.y.todouble() - a.max.y.todouble()), 2));
	fixp distp = fixp_distance(a.min, a.max);
	assert(distp >= distf - distf * 0.1f && distp <= distf + distf * 0.1f);
	double lenf = sqrt(dotf(a.max, a.max));
	fixp lenp = fixp_length(a.max);
	assert(lenp >= lenf - lenf * 0.01f && lenp <= lenf + lenf * 0.01f);
	double normf1  = a.max.x.todouble() / sqrt(dotf(a.max, a.max));
	double normf2  = a.max.y.todouble() / sqrt(dotf(a.max, a.max));
	ivec2 normp = fixp_normal(a.max);
	assert(normp.x == normf1);
	assert(normp.y == normf2);

	return 0;
}
