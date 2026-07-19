#include "dmath.h"

#include <assert.h>
#include <stdint.h>

int main()
{
	// b ahead of a
	assert(circular_distance(4, 5) == 1);
	assert(circular_distance(0, 100) == 100);

	// b behind a
	assert(circular_distance(5, 4) == -1);
	assert(circular_distance(100, 0) == -100);

	// same point
	assert(circular_distance(0, 0) == 0);
	assert(circular_distance(42, 42) == 0);

	// wraparound: 0 is 1 ahead of UINT32_MAX
	assert(circular_distance(UINT32_MAX, 0) == 1);

	// wraparound going backward: UINT32_MAX is 1 behind 0
	assert(circular_distance(0, UINT32_MAX) == -1);

	// wraparound spanning multiple steps
	assert(circular_distance(UINT32_MAX - 1, 1) == 3);
	assert(circular_distance(1, UINT32_MAX - 1) == -3);

	// exactly halfway: convention is negative (treated as behind)
	assert(circular_distance(0, (uint32_t)INT32_MIN) == INT32_MIN);

	// lfsr_tap edge cases and coverage
	assert(lfsr_tap(2) == 0x3);
	assert(lfsr_tap(32) == 0x80000057);
	for (uint32_t size = 2; size <= 32; ++size)
	{
		assert(lfsr_tap(size) != 0);
	}

	return 0;
}
