#include "dice.h"
#include <assert.h>
#include <stdio.h>

void test_quadratic(seed s, int c)
{
	const int n = 50000;
	printf("quadratic_weighted_roll(%d) outcomes from %d rolls:\n", c, n);
	std::vector<int> r(n);
	for (auto& i : r)
	{
		i = s.quadratic_weighted_roll(c);
		if (i > c || 0 > i) printf("%d\n", i);
		assert(i <= c && i >= 0);
	}
	for (int i = 0; i <= c; i++)
	{
		int count = 0;
		for (const int j : r) { assert(j <= c && j >= 0); if (j == i) count++; }
		printf("\t%d => %d\n", i, count);
	}
}

void test_pow2(seed s, int c)
{
	const int n = 50000;
	printf("pow2_weighted_roll(%d) outcomes from %d rolls:\n", c, n);
	std::vector<int> r(n);
	for (auto& i : r)
	{
		i = s.pow2_weighted_roll(c);
		if (i > c || 0 > i) printf("%d\n", i);
		assert(i <= c && i >= 0);
	}
	for (int i = 0; i <= c; i++)
	{
		int count = 0;
		for (const int j : r) { assert(j <= c && j >= 0); if (j == i) count++; }
		const int lim_low = n / (1<<(i+1)) -  (n/10) / (1<<(i+1));
		const int lim_high = n / (1<<(i+1)) + (n/10) / (1<<(i+1));
		printf("\t%d => %d (within (%d, %d)\n", i, count, lim_low, lim_high);
		assert(count > lim_low && count < lim_high); // be accurate within 10%
	}
}

void test_roll(seed s, int c)
{
	const int n = 50000;
	printf("roll(0, %d) outcomes from %d rolls:\n", c, n);
	std::vector<int> r(n);
	for (auto& i : r)
	{
		i = s.roll(0, c);
		if (i > c || 0 > i) printf("%d\n", i);
		assert(i <= c && i >= 0);
	}
	for (int i = 0; i <= c; i++)
	{
		int count = 0;
		for (const int j : r) { assert(j <= c && j >= 0); if (j == i) count++; }
		const int lim_low = n / (c+1) - (n/10) / (c+1);
		const int lim_high = n / (c+1) + (n/10) / (c+1);
		printf("\t%d => %d (within (%d, %d)\n", i, count, lim_low, lim_high);
		assert(count > lim_low && count < lim_high); // be accurate within 10%
	}
}

int main(int argc, char **argv)
{
	test_roll(seed(64), 4);
	test_quadratic(seed(64), 4);
	test_pow2(seed(64), 4);
	return 0;
}
