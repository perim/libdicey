#include "direction.h"

#include <assert.h>
#include <stdio.h>
#include <cstdint>
#include <cmath>

int main()
{
	int32_t max = 0, min = 0;
	for (int i = 0; i <= UINT16_MAX; i++)
	{
		int s = lround(4096 * sin(2 * M_PI * i / UINT16_MAX));
		int s5d = isin(i);
		int err = s - s5d;
		if (err > max) max = err;
		if (err < min) min = err;
	}
	printf("min: %i max: %i\n", min, max);
	assert(min <= 2 && max <= 2);

	for (int i = 0; i <= UINT16_MAX; i++)
	{
		int c = lround(4096 * cos(2 * M_PI * i / UINT16_MAX));
		int c5d = icos(i);
		int err = c - c5d;
		if (err > max) max = err;
		if (err < min) min = err;
	}
	printf("min: %i max: %i\n", min, max);
	assert(min <= 2 && max <= 2);

	return 0;
}
