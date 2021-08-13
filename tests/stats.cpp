#include "dice.h"
#include <assert.h>
#include <stdio.h>

void test_prd1(seed s, int c)
{
	const int n = 10000;
	printf("prd(s, %d, prd_function::predictable) outcomes from %d rolls:\n", c, n);
	prd p(s, c, prd_function::predictable);
	std::vector<int> r(n);
	for (auto& i : r)
	{
		i = p.roll();
		assert(i == 0 || i == 1);
	}
	int count = 0;
	for (const int j : r) { assert(j == 1 || j == 0); if (j == 1) count++; }
	const int lim_low = n / 1000 * c - n / 10000 * c;
	const int lim_high = n / 1000 * c + n / 10000 * c;
	printf("\ttrue => %d (should be %d, within %d, %d)\n", count, (n / 1000 * c), lim_low, lim_high);
	assert(count >= lim_low && count <= lim_high);
}

void test_prd2(seed s, int c)
{
	const int n = 10000;
	printf("prd(s, %d, prd_function::fair) outcomes from %d rolls:\n", c, n);
	prd p(s, c, prd_function::fair);
	std::vector<int> r(n);
	for (auto& i : r)
	{
		i = p.roll();
		assert(i == 0 || i == 1);
	}
	int count = 0;
	for (const int j : r) { assert(j == 1 || j == 0); if (j == 1) count++; }
	const int lim_low = n / 1000 * c - n / 10000 * c;
	const int lim_high = n / 1000 * c + n / 10000 * c;
	printf("\ttrue => %d (should be %d, within %d, %d)\n", count, (n / 1000 * c), lim_low, lim_high);
	assert(count >= lim_low && count <= lim_high);
}

void test_prd3(seed s, int c)
{
	const int n = 10000;
	printf("prd(s, %d, prd_function::relaxed) outcomes from %d rolls:\n", c, n);
	prd p(s, c, prd_function::relaxed);
	std::vector<int> r(n);
	for (auto& i : r)
	{
		i = p.roll();
		assert(i == 0 || i == 1);
	}
	int count = 0;
	for (const int j : r) { assert(j == 1 || j == 0); if (j == 1) count++; }
	const int lim_low = n / 1000 * c - n / 10000 * c;
	const int lim_high = n / 1000 * c + n / 10000 * c;
	printf("\ttrue => %d (should be %d, within %d, %d)\n", count, (n / 1000 * c), lim_low, lim_high);
	assert(count >= lim_low && count <= lim_high);
}

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

void test_roll_luck(seed s, int c, int min, luck_type l)
{
	const int n = 50000;
	if (l == luck_type::lucky) printf("roll(%d, %d, %d) outcomes from %d lucky rolls:\n", min, c, (int)l, n);
	else if (l == luck_type::unlucky) printf("roll(%d,  %d, %d) outcomes from %d unlucky rolls:\n", min, c, (int)l, n);
	else if (l == luck_type::very_lucky) printf("roll(%d, %d, %d) outcomes from %d very lucky rolls:\n", min, c, (int)l, n);
	else if (l == luck_type::very_unlucky) printf("roll(%d, %d, %d) outcomes from %d very unlucky rolls:\n", min, c, (int)l, n);
	else if (l == luck_type::mediocre) printf("roll(%d, %d, %d) outcomes from %d mediocre rolls:\n", min, c, (int)l, n);
	else if (l == luck_type::uncommon) printf("roll(%d, %d, %d) outcomes from %d uncommon rolls:\n", min, c, (int)l, n);
	std::vector<int> r(n);
	for (auto& i : r)
	{
		i = s.roll(min, c, l);
		if (i > c || 0 > i) printf("%d\n", i);
		assert(i <= c && i >= 0);
	}
	int sum = 0;
	for (int i = min; i <= c; i++)
	{
		int count = 0;
		for (const int j : r) { assert(j <= c && j >= 0); if (j == i) count++; }
		sum += count;
		printf("\t%d => %d\n", i, count);
	}
	assert(sum == n);
}

int main(int argc, char **argv)
{
	test_roll(seed(64), 4);
	test_roll_luck(seed(64), 4, 0, luck_type::lucky);
	test_roll_luck(seed(64), 4, 0, luck_type::unlucky);
	test_roll_luck(seed(64), 4, 0, luck_type::very_lucky);
	test_roll_luck(seed(64), 4, 0, luck_type::very_unlucky);
	test_roll_luck(seed(64), 4, 0, luck_type::mediocre);
	test_roll_luck(seed(64), 4, 0, luck_type::uncommon);
	test_roll_luck(seed(64), 8, 4, luck_type::mediocre);
	test_roll_luck(seed(64), 8, 4, luck_type::uncommon);
	test_quadratic(seed(64), 4);
	test_pow2(seed(64), 4);

	test_prd1(seed(4), 495);
	test_prd1(seed(64), 505);
	test_prd1(seed(164), 1);
	test_prd1(seed(264), 10);
	test_prd1(seed(364), 100);
	test_prd1(seed(4641), 999);

	test_prd3(seed(264), 500);
	test_prd3(seed(364), 1);
	test_prd3(seed(464), 495);
	test_prd3(seed(564), 10);
	test_prd3(seed(664), 30);
	test_prd3(seed(764), 900);
	test_prd3(seed(864), 999);

	test_prd2(seed(264), 495);
	test_prd2(seed(364), 1);
	test_prd2(seed(464), 10);
	test_prd2(seed(564), 100);
	test_prd2(seed(664), 999);

	return 0;
}
