#include "dice.h"
#include <assert.h>
#include <stdio.h>
#include <inttypes.h>

// test performance of the roll table rolls() call
int main(int argc, char **argv)
{
	seed s(1);

	uint64_t t1 = cpu_gettime();
	uint64_t sum = 0;
	const std::vector<int> t { 1, 2, 3, 4 };
	roll_table rt(s, t);
	int results[4];
	for (int i = 1; i < 50000; i++)
	{
		rt.rolls(4, results, luck_type::normal, 0);
		sum += results[0];
	}
	uint64_t t2 = cpu_gettime();
	printf("%-30s %'12" PRIu64 "\n", "roll_table", t2 - t1);

	linear_roll_table lrt8(s, 8);
	linear_series ls8(s, 8);
	printf("Size of 8 entry LRT=%u LS=%u\n", (unsigned)(sizeof(lrt8) + lrt8.table.size() * sizeof(*lrt8.table.data())), (unsigned)sizeof(ls8));

	linear_roll_table lrt5k(s, 5000);
	linear_series ls5k(s, 5000);
	printf("Size of 5000 entry LRT=%u LS=%u\n", (unsigned)(sizeof(lrt5k) + lrt5k.table.size() * sizeof(*lrt5k.table.data())), (unsigned)sizeof(ls5k));

	t1 = cpu_gettime();
	for (int i = 0; i < 4000; i++) lrt5k.roll();
	t2 = cpu_gettime();
	printf("%-30s %'12" PRIu64 "\n", "LRT 4k rolls 5k alloc", t2 - t1);

	t1 = cpu_gettime();
	for (int i = 0; i < 4000; i++) ls5k.roll();
	t2 = cpu_gettime();
	printf("%-30s %'12" PRIu64 "\n", "LS 4k rolls 5k alloc", t2 - t1);

	const std::vector<int> w5k(5000, 1);
	roll_table rt5k(s, w5k);
	t1 = cpu_gettime();
	for (int i = 0; i < 4000; i++) rt5k.roll();
	t2 = cpu_gettime();
	printf("%-30s %'12" PRIu64 "\n", "RT 4k rolls 5k alloc", t2 - t1);

	linear_series ls4k(s, 4096-1);
	t1 = cpu_gettime();
	for (int i = 0; i < 4000; i++) ls4k.roll();
	t2 = cpu_gettime();
	printf("%-30s %'12" PRIu64 "\n", "LS 4k rolls 4k-1 alloc", t2 - t1);

	std::vector<int> weights(200);
	for (int i = 0; i < 200; i++) weights[i] = 100 + i*50;
	const_roll_table crt(weights);
	roll_table drt(s, weights);
	sum = 0;
	t1 = cpu_gettime();
	for (int i = 0; i < 400000; i++) sum += crt.roll(s);
	t2 = cpu_gettime();
	printf("%-30s %'12" PRIu64 " sum=%" PRIu64 "\n", "CRT 400k rolls", t2 - t1, sum);
	sum = 0;
	t1 = cpu_gettime();
	for (int i = 0; i < 400000; i++) sum += drt.roll();
	t2 = cpu_gettime();
	printf("%-30s %'12" PRIu64 " sum=%" PRIu64 "\n", "DRT 400k rolls", t2 - t1, sum);

	return (int)sum * 0;
}
