#include "dice.h"
#include <assert.h>
#include <stdio.h>

// test performance of the roll() call
int main(int argc, char **argv)
{
	seed s = seed_random();
	uint64_t sum = 0;
	for (int i = 1; i < 500000; i++)
	{
		sum += s.roll(i >> 2, i);
	}
	return (int)sum * 0;
}
