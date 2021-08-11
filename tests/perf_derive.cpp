#include "dice.h"
#include <assert.h>
#include <stdio.h>

// test performance of the derive() call
int main(int argc, char **argv)
{
	seed s = seed_random();
	uint64_t sum = 0;
	for (int i = 1; i < 50000; i++)
	{
		const seed s2 = s.derive(i, i);
		sum += s2.state;
	}
	return (int)sum * 0;
}
