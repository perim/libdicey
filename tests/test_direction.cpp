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

	for (int i = 0; i <= UINT16_MAX; i++)
	{
		direction expected = 0;
		if (i >= 8192 && i <= 24575) expected = 16384;
		else if (i >= 24576 && i <= 40959) expected = 32768;
		else if (i >= 40960 && i <= 57343) expected = 49152;

		if (snap(i) != expected) {
			printf("snap(%d) failed. Expected %d, got %d\n", i, expected, snap(i));
		}
		assert(snap(i) == expected);
	}
	printf("snap tests passed\n");

	return 0;
}
