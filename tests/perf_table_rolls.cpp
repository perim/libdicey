#include "dice.h"
#include <assert.h>
#include <stdio.h>

// test performance of the roll table rolls() call
int main(int argc, char **argv)
{
	seed s = seed_random();
	uint64_t sum = 0;
	const std::vector<int> t { 1, 2, 3, 4 };
	roll_table rt(s, t);
	int results[4];
	for (int i = 1; i < 50000; i++)
	{
		rt.rolls(4, results, luck_type::normal, 0);
		sum += results[0];
	}
	return (int)sum * 0;
}
