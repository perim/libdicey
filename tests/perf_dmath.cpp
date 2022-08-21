#include "dmath.h"
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <algorithm>
#include <limits>
#include <array>

static uint64_t a = 0;

int main(int argc, char **argv)
{
	lazy_conditional_matrix<128, 128> m;
	for (int i = 0; i < 128; i++)
	{
		for (int j = 0; j < 128; j++)
		{
			m.modify(i, j, 140);
		}
	}
	for (int i = 0; i < 128; i++)
	{
		for (int j = 0; j < 128; j++) a += m.row(j);
		for (int j = 0; j < 128; j++) a += m.row(j);
		for (int j = 0; j < 128; j++) a += m.row(j);
	}

	uint64_t sum = 0;
	for (int i = 1; i < 50000; i++)
	{
		sum += isqrt(i);
	}
	return (int)sum * 0;
}
