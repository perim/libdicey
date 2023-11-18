#include "dice.h"
#include <assert.h>
#include <stdio.h>

static bool nearly_equalf(double a, double b)
{
	return (fabs(a - b) < 0.2 && fabs(a - b) < 0.2);
}

static bool nearly_equali(int a, int b)
{
	return (a - 1 <= b && a + 1 >= b);
}

static void linear_roll_table_test()
{
	const int len1 = 52;
	std::vector<int> results(len1, 0);
	seed s(0);
	// deck policy test
	linear_roll_table lrt1(s, len1);
	for (int i = 0; i < len1; i++) results[lrt1.roll()]++;
	for (int i = 0; i < len1; i++) assert(results[i] == 1);
	for (int i = 0; i < len1; i++) results[lrt1.roll()]--;
	for (int i = 0; i < len1; i++) assert(results[i] == 0);
	lrt1.reset();
	lrt1.reset();
	for (int i = 0; i < len1; i++) results[lrt1.roll()]++;
	for (int i = 0; i < len1; i++) assert(results[i] == 1);
	for (int i = 0; i < len1 * 2; i++) results[lrt1.roll()]++;
	for (int i = 0; i < len1; i++) assert(results[i] == 3);
	// repeat policy test
	linear_roll_table lrt2(s, len1, empty_table_policy::repeat_first);
	std::fill(results.begin(), results.end(), 0);
	for (int i = 0; i < len1; i++) results[lrt2.roll()]++;
	for (int i = 0; i < len1; i++) assert(results[i] == 1);
	assert(lrt2.roll() == 0);
	assert(lrt2.roll() == 0);
	lrt2.reset();
	std::fill(results.begin(), results.end(), 0);
	for (int i = 0; i < len1; i++) results[lrt2.roll()]++;
	for (int i = 0; i < len1; i++) assert(results[i] == 1);
	assert(lrt2.roll() == 0);
	// minus one policy test
	linear_roll_table lrt3(s, len1, empty_table_policy::return_minus_one);
	std::fill(results.begin(), results.end(), 0);
	for (int i = 0; i < len1; i++) results[lrt3.roll()]++;
	for (int i = 0; i < len1; i++) assert(results[i] == 1);
	assert(lrt3.roll() == -1);
	assert(lrt3.roll() == -1);
	lrt3.reset();
	std::fill(results.begin(), results.end(), 0);
	for (int i = 0; i < len1; i++) results[lrt3.roll()]++;
	for (int i = 0; i < len1; i++) assert(results[i] == 1);
	assert(lrt3.roll() == -1);
	// add test
	std::fill(results.begin(), results.end(), 0);
	linear_roll_table lrt4(s, len1, empty_table_policy::return_minus_one, 40);
	for (int i = 0; i < 40; i++) { int r = lrt4.roll(); assert(r != -1); results[r]++; }
	for (int i = 0; i < 40; i++) assert(results[i] == 1);
	for (int i = 41; i < len1; i++) assert(results[i] == 0);
	bool b = lrt4.add(50); assert(b);
	b = lrt4.add(40); assert(b);
	std::fill(results.begin(), results.end(), 0);
	lrt4.reset();
	for (int i = 0; i < 42; i++) { int r = lrt4.roll(); assert(r != -1); results[r]++; }
	for (int i = 0; i < 40; i++) assert(results[i] == 1);
	assert(results[40] == 1);
	assert(results[41] == 0);
	assert(results[50] == 1);
	assert(results[51] == 0);
	// remove test
	std::fill(results.begin(), results.end(), 0);
	lrt4.reset();
	int r = lrt4.roll();
	lrt4.remove();
	for (int i = 0; i < 41; i++) { int r = lrt4.roll(); assert(r != -1); results[r]++; }
	for (int i = 0; i < 40; i++) assert(results[i] == 1 || i == r);
	assert(results[r] == 0);
	assert(lrt4.roll() == -1);
	lrt4.reset();
	for (int i = 0; i < 42; i++) { lrt4.roll(); lrt4.remove(); } // remove all
	assert(lrt4.roll() == -1);
	lrt4.reset();
	assert(lrt4.roll() == -1);
}

static void perten_test()
{
	for (int i = 0; i < 2000; i++) { float f = i * 0.1f; assert(nearly_equalf(f, perten_to_percentf(percent_to_perten(f)))); }
	for (int i = 0; i < 16400; i++) { assert(i == perten_to_percent(percent_to_perten(i))); }
	for (perten i = 0; i < UINT16_MAX; i++) { perten v = percent_to_perten(perten_to_percentf(i)); assert(nearly_equali(i, v)); }
	assert(perten_add_increase(perten_base, perten_base) == perten_base * 2);
	assert(perten_add_increase(perten_base, perten_base * 2) == perten_base * 3);
	assert(perten_add_reduction(100, perten_base / 2) == 50);
	assert(perten_apply(100, 0) == 0);
	assert(perten_apply(100, perten_base) == 100);
	assert(perten_apply(100, perten_base * 2) == 200);
	assert(perten_apply(100, perten_base / 2) == 50);
	assert(perten_apply(100, UINT16_MAX) == 6399);
	assert(perten_apply(perten_base, perten_base + 1) == perten_base + 1);
	assert(perten_apply(UINT32_MAX, perten_base + 1) == UINT32_MAX);
	assert(perten_apply(UINT32_MAX, UINT32_MAX) == UINT32_MAX);
}

static void test_condmatrix()
{
	lazy_conditional_matrix<16, 16> matrix;
	for (int i = 0; i < 16; i++) assert(matrix.is_enabled(i) == false);
	for (int i = 0; i < 16; i++) assert(matrix.row(i) == perten_base); // cached zero value
	matrix.modify(0, 0, 16);
	assert(matrix.row(0) == perten_base); // not yet enabled
	matrix.toggle(0, true);
	assert(matrix.row(0) == 16); // now 16
	assert(matrix.row(0) == 16); // cached 16
	matrix.modify(1, 0, perten_base * 2);
	matrix.modify(2, 0, perten_base * 2);
	matrix.modify(3, 0, 32);
	matrix.modify(4, 0, 32);
	matrix.toggle(0, true);
	matrix.toggle(1, true);
	assert(matrix.row(0) == 32);
}

static void edge_cases()
{
	seed s(0);
	seed s2(1);
	assert(s.roll(1, 100) != s2.roll(1, 100));
	assert(s.state != s2.state);
	(void)s.roll(10, 9); // just don't crash, return value will be nonsensical
	int r = s.roll(10, 10);
	assert(r == 10);
	r = s.roll(0, 0);
	assert(r == 0);
	r = s.roll(-10, -10);
	assert(r == -10);
	r = s.roll(-1, 1);
	assert(r >= -1 && r <= 1);
	r = s.roll(-10, -1);
	assert(r <= -1 && r >= -10);
	r = s.pow2_weighted_roll(0);
	assert(r == 0);
	r = s.quadratic_weighted_roll(0);
	assert(r == 0);

	prd p1(s, 0, prd_function::fair);
	assert(p1.roll() == false);
	prd p2(s, 0, prd_function::relaxed);
	assert(p2.roll() == false);
	prd p3(s, 0, prd_function::predictable);
	assert(p3.roll() == false);

	prd p4(s, 1000, prd_function::fair);
	assert(p4.roll() == true);
	prd p5(s, 1000, prd_function::relaxed);
	assert(p5.roll() == true);
	prd p6(s, 1000, prd_function::predictable);
	assert(p6.roll() == true);
}

static void test_linear_series_1()
{
	seed s(1);
	linear_series ls(s, 2);
	for (unsigned i = 0; i < 12; i++)
	{
		assert(ls.remaining() == 2);
		assert(ls.size() == 2);
		unsigned v1 = ls.roll();
		assert(ls.remaining() == 1);
		assert(ls.size() == 2);
		unsigned v2 = ls.roll();
		assert(ls.remaining() == 2);
		assert(ls.size() == 2);
		assert(v1 != v2);
		assert(v1 == 0 || v1 == 1);
		assert(v2 == 0 || v2 == 1);
	}
}
static void test_linear_series_2()
{
	seed s(1);
	linear_series ls(s, 3);
	for (unsigned i = 0; i < 12; i++)
	{
		assert(ls.remaining() == 3);
		assert(ls.size() == 3);
		unsigned v1 = ls.roll();
		assert(ls.remaining() == 2);
		assert(ls.size() == 3);
		assert(ls.reserved() == 4);
		unsigned v2 = ls.roll();
		assert(ls.remaining() == 1);
		unsigned v3 = ls.roll();
		assert(ls.remaining() == 3);
		assert(v1 != v2);
		assert(v1 != v3);
		assert(v2 != v3);
		assert(v1 == 0 || v2 == 0 || v3 == 0);
		assert(v1 == 1 || v2 == 1 || v3 == 1);
		assert(v1 == 2 || v2 == 2 || v3 == 2);
	}
}
static void test_linear_series_3()
{
	seed s(1);
	linear_series ls(s, 4);
	for (unsigned i = 0; i < 12; i++)
	{
		assert(ls.remaining() == 4);
		assert(ls.size() == 4);
		unsigned v1 = ls.roll();
		assert(ls.remaining() == 3);
		assert(ls.size() == 4);
		assert(ls.reserved() == 8);
		unsigned v2 = ls.roll();
		assert(ls.remaining() == 2);
		unsigned v3 = ls.roll();
		assert(ls.remaining() == 1);
		unsigned v4 = ls.roll();
		assert(ls.remaining() == 4);
		assert(v1 != v2);
		assert(v1 != v3);
		assert(v1 != v4);
		assert(v3 != v4);
		assert(v1 == 0 || v2 == 0 || v3 == 0 || v4 == 0);
		assert(v1 == 1 || v2 == 1 || v3 == 1 || v4 == 1);
		assert(v1 == 2 || v2 == 2 || v3 == 2 || v4 == 2);
		assert(v1 == 3 || v2 == 3 || v3 == 3 || v4 == 3);
	}
}

static void test_const_roll_table_1()
{
	std::vector<int> w{ 50, 50, 100, 100 };
	const_roll_table r(w);
	assert(r.sum == 300);
	assert(r.probability.size() == 4);
	assert(r.alias.at(2) == -1);
	assert(r.alias.at(3) == -1);
}

static void test_const_roll_table_2()
{
	std::vector<int> w{ 50, 50, 100 };
	const_roll_table r(w);
	assert(r.sum == 200);
	assert(r.probability.size() == 3);
	assert(r.alias.at(2) == -1);
}

static void test_const_roll_table_3()
{
	std::vector<int> w{ 50, 50 };
	const_roll_table r(w);
	assert(r.sum == 100);
	assert(r.probability.size() == 2);
	assert(r.alias.at(0) == -1);
	assert(r.alias.at(1) == -1);
}

static void test_const_roll_table_4()
{
	std::vector<int> w{ 50 };
	const_roll_table r(w);
	assert(r.sum == 50);
	assert(r.probability.size() == 1);
	assert(r.alias.at(0) == -1);
}

static void test_const_roll_table_5()
{
	seed s(1);
	std::vector<int> weights(200);
	std::vector<int> results(200);
	for (int i = 0; i < 200; i++) weights[i] = 100 + i*50;
	const_roll_table crt(weights);
	assert(crt.alias.size() == 200);
	for (int i = 0; i < 200; i++) { int v = crt.roll(s); assert(v < 200); }
	for (int i = 0; i < 200 * 1024; i++) results.at(crt.roll(s))++;
	//for (int i = 0; i < 200; i++) printf("%d : %f == %f\n", i, (double)results.at(i) / (200.0 * 1024.0), (double)weights.at(i) / (double)crt.sum);
}

int main(int argc, char **argv)
{
	test_const_roll_table_1();
	test_const_roll_table_2();
	test_const_roll_table_3();
	test_const_roll_table_4();
	test_const_roll_table_5();
	test_linear_series_1();
	test_linear_series_2();
	test_linear_series_3();
	linear_roll_table_test();
	perten_test();
	edge_cases();
	test_condmatrix();

	int j = 0;
	for (unsigned i = 1; i < (1 << 12); i <<= 1)
	{
		assert(highestbitset(i) == j);
		assert(ispow2(i));
		assert(i == 2 || !ispow2(i - 1));
		if (i > 1) assert(next_pow2(i) == i);
		j++;
	}

	seed s = seed_random();
	int r1 = s.roll(0, 4);
	assert(r1 <= 4 && r1 >= 0);
	r1 = s.roll(4, 16);
	assert(r1 <= 16 && r1 >= 4);
	assert(s.state != s.orig);
	int r2 = s.roll(0, 4, luck_type::normal);
	assert(r2 <= 4 && r2 >= 0);
	int r3 = s.roll(0, 4, luck_combine(luck_type::normal, luck_type::normal), 1, 2, 5, luck_type::normal);
	assert(r3 >= 0);
	int result = 0;
	std::vector<int> input { 1, 1, 3, 2 };
	roll_table rt(s, input);
	int r6 = rt.rolls(1, &result);
	assert(r6 == 1);
	assert(result >= 0 && result <= 3);
	r6 = rt.unique_rolls(1, &result);
	assert(r6 == 1);
	assert(result >= 0 && result <= 3);
	int res[10];
	r6 = rt.rolls(10, res);
	assert(r6 == 10);
	assert(res[0] >= 0 && res[0] <= 3);
	assert(res[1] >= 0 && res[1] <= 3);
	assert(res[2] >= 0 && res[2] <= 3);
	assert(res[9] >= 0 && res[9] <= 3);
	r6 = rt.unique_rolls(10, res);
	assert(r6 == 4);
	assert(res[0] >= 0 && res[0] <= 3);
	assert(res[3] >= 0 && res[3] <= 3);
	assert(res[0] != res[3] && res[0] != res[1] && res[1] != res[2]);

	seed ds1 = s.derive(0, 1);
	seed ds2 = s.derive(1, 0); assert(ds2.state != ds1.state);
	seed ds3 = s.derive(1, 1); assert(ds3.state != ds2.state);
	seed ds4 = s.derive(1, 3); assert(ds4.state != ds3.state);
	seed ds5 = s.derive(0, 3); assert(ds5.state != ds4.state);
	assert(s.derive(0, 10).state != s.derive(10, 0).state);

	seed s2 = s.derive(15);
	seed s22 = s.derive(15);
	assert(s2.state == s22.state && s2.orig == s22.orig);
	int r7 = s2.roll(1, 10);
	assert(s2.state != s2.orig && s2.state != s.state);
	int r77 = s22.roll(1, 10);
	assert(r7 == r77);
	assert(s2.state == s22.state && s2.orig == s22.orig);

	seed s3 = s.derive(15, 15);
	int r8 = s3.roll(1, 10);
	assert(r8 >= 1 && r8 <= 10);

	assert(luck_combine(luck_type::normal, luck_type::normal) == luck_type::normal);
	assert(luck_combine(luck_type::lucky, luck_type::normal) == luck_type::lucky);
	assert(luck_combine(luck_type::normal, luck_type::lucky) == luck_type::unlucky);

	roll_table cpy(rt);
	r8 = cpy.boxgacha(luck_type::lucky, 25);
	assert(r8 >= 0);
	r8 = cpy.boxgacha();
	assert(r8 >= 0);
	r8 = cpy.boxgacha();
	assert(r8 >= 0);
	r8 = cpy.boxgacha();
	assert(r8 >= 0);
	r8 = cpy.boxgacha();
	assert(r8 == -1);

	prd p(s, 100);
	assert(p.value == true);
	(void)p.roll();
	(void)p.roll(luck_type::lucky);

	// testing the 'fair' function
	for (int i = 0; i < 500; i += 5)
	{
		seed s9 = seed_random();
		prd p2(s9, 100, prd_function::fair); // 10% chance
		int j = 0;
		for (; j < 5; j++) assert(p2.roll() == false); // first 5 shall never be true
		while (!p2.roll()) j++;
		for (; j < 5; j++) assert(p2.roll() == false); // next 5 shall never be true
	}

	// testing the 'predictable' function
	for (int i = 5; i < 500; i += 5)
	{
		prd p2(s, i, prd_function::predictable);
		int c = 0;
		const int ceiling = 1000 / i;
		while (!p2.roll())
		{
			c++;
			assert(c <= ceiling);
		}
		assert(p2.remainder <= ceiling);
		assert(p2.accum <= ceiling);
	}
	for (int i = 505; i < 1000; i += 5)
	{
		prd p2(s, i, prd_function::predictable);
		int c = 0;
		const int ceiling = 1000 / (1000 - i);
		while (p2.roll())
		{
			c++;
			assert(c <= ceiling);
		}
		assert(p2.remainder <= ceiling);
		assert(p2.accum <= ceiling);
	}

	for (int i = 0; i <= UINT16_MAX; i++)
	{
		const uint64_t i1 = sqrt(i);
		const uint64_t i2 = isqrt(i);
		const uint32_t i3 = isqrt32(i);
		assert(i1 == i2 && i1 == i3);
	}

	assert(next_pow2(2) == 2);
	assert(next_pow2(3) == 4);
	assert(next_pow2(5) == 8);

	assert(range_overlap(0, 1, 1, 2));
	assert(!range_overlap(0, 1, 2, 3));
	assert(range_overlap(3, 9, 4, 11));

	return 0;
}
