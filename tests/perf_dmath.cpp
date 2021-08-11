#include "dmath.h"
#include <assert.h>
#include <stdio.h>

int main(int argc, char **argv)
{
	uint64_t sum = 0;
	for (int i = 1; i < 50000; i++)
	{
		sum += isqrt(i);
	}
	return (int)sum * 0;
}
