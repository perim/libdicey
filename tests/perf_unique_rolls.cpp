#include "dice.h"
#include <assert.h>
#include <stdio.h>

// test performance of the unique_rolls() call
int main(int argc, char **argv)
{
	seed s = seed_random();
	uint64_t sum = 0;
	const std::vector<int> t { 1, 2, 3, 4 };
	roll_table rt(s, t);
	int results[4];
	for (int i = 1; i < 50000; i++)
	{
		// int unique_rolls(const roll_table* table, int count, int* results, luck_type rollee = luck_type::normal, int roll_weight = 0, int start_index = 0);
		rt.unique_rolls(4, results, luck_type::normal, 0, 0);
		sum += results[0];
	}
	return (int)sum * 0;
}
