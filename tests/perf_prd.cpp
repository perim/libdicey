#include "dice.h"
#include <assert.h>
#include <stdio.h>

// test performance of the prd class
int main(int argc, char **argv)
{
	seed s = seed_random();
	prd p(s, 15); // 1.5% chance
	uint64_t sum = 0;
	for (int i = 1; i < 50000; i++)
	{
		if (p.roll()) sum++;
	}
	return (int)sum * 0;
}
