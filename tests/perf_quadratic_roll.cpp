#include "dice.h"

int main(int argc, char **argv)
{
	seed s = seed_random();
	uint64_t sum = 0;
	for (int i = 1; i < 50000; i++)
	{
		sum += s.quadratic_weighted_roll(21);
	}
	return (int)sum * 0;
}
