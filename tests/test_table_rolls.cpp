#include "dice.h"
#include <assert.h>
#include <stdio.h>

// test performance of the roll table rolls() call
int main(int argc, char **argv)
{
	seed s = seed_random();
	const std::vector<int> t { 1, 10, 100, 1000 };
	roll_table rt(s, t);
	int results[4] = { 0, 0, 0, 0 };
	int result;
	for (int i = 1; i < 50000; i++)
	{
		rt.rolls(1, &result, luck_type::normal, 0);
		results[result]++;
	}
	printf("weight %d: %d\n", t[0], results[0]);
	printf("weight %d: %d\n", t[1], results[1]);
	printf("weight %d: %d\n", t[2], results[2]);
	printf("weight %d: %d\n", t[3], results[3]);
	return 0;
}
