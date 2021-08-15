#include "dice.h"
#include <assert.h>
#include <stdio.h>

static void edge_cases()
{
	seed s(10);
	seed s2(11);
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

int main(int argc, char **argv)
{
	edge_cases();

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
	roll_table* rt = roll_table_make(input);
	int r6 = s.rolls(rt, 1, &result);
	assert(r6 == 1);
	assert(result >= 0 && result <= 3);
	r6 = s.unique_rolls(rt, 1, &result);
	assert(r6 == 1);
	assert(result >= 0 && result <= 3);
	int res[10];
	r6 = s.rolls(rt, 10, res);
	assert(r6 == 10);
	assert(res[0] >= 0 && res[0] <= 3);
	assert(res[1] >= 0 && res[1] <= 3);
	assert(res[2] >= 0 && res[2] <= 3);
	assert(res[9] >= 0 && res[9] <= 3);
	r6 = s.unique_rolls(rt, 10, res);
	assert(r6 == 4);
	assert(res[0] >= 0 && res[0] <= 3);
	assert(res[3] >= 0 && res[3] <= 3);
	assert(res[0] != res[3] && res[0] != res[1] && res[1] != res[2]);

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

	roll_table* cpy = roll_table_copy(rt);
	r8 = s.boxgacha(cpy, luck_type::lucky, 25);
	assert(r8 >= 0);
	r8 = s.boxgacha(cpy);
	assert(r8 >= 0);
	r8 = s.boxgacha(cpy);
	assert(r8 >= 0);
	r8 = s.boxgacha(cpy);
	assert(r8 >= 0);
	r8 = s.boxgacha(cpy);
	assert(r8 == -1);
	roll_table_free(cpy);

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

	roll_table_free(rt);

	for (int i = 0; i <= UINT16_MAX; i++)
	{
		const uint64_t i1 = sqrt(i);
		const uint64_t i2 = isqrt(i);
		const uint32_t i3 = isqrt32(i);
		assert(i1 == i2 && i1 == i3);
	}

	int j = 0;
	for (unsigned i = 1; i < (1 << 12); i <<= 1)
	{
		assert(highestbitset(i) == j);
		assert(ispow2(i));
		assert(i == 2 || !ispow2(i - 1));
		j++;
	}

	assert(range_overlap(0, 1, 1, 2));
	assert(!range_overlap(0, 1, 2, 3));
	assert(range_overlap(3, 9, 4, 11));

	return 0;
}
