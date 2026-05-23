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

	const int distances[] = {0, 1, 10, 100, 1000, 10000, 65536, 100000, 1000000};
	for (int dist : distances)
	{
		int32_t rmax = 0, rmin = 0;
		for (int i = 0; i <= UINT16_MAX; i++)
		{
			double exact = dist * sin(2 * M_PI * i / 65536.0) / 16.0;
			int s = lround(exact);
			int s5d = isinr(i, dist);
			int err = s - s5d;
			if (err > rmax) rmax = err;
			if (err < rmin) rmin = err;
		}
		int limit = dist > 0 ? (dist * 2 / 65536) + 2 : 0;
		printf("isinr dist %i min: %i max: %i (limit: %i)\n", dist, rmin, rmax, limit);
		assert(rmin >= -limit && rmax <= limit);

		rmax = 0; rmin = 0;
		for (int i = 0; i <= UINT16_MAX; i++)
		{
			double exact = dist * cos(2 * M_PI * i / 65536.0) / 16.0;
			int c = lround(exact);
			int c5d = icosr(i, dist);
			int err = c - c5d;
			if (err > rmax) rmax = err;
			if (err < rmin) rmin = err;
		}
		printf("icosr dist %i min: %i max: %i (limit: %i)\n", dist, rmin, rmax, limit);
		assert(rmin >= -limit && rmax <= limit);
	}
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
