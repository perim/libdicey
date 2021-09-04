#pragma once

// Fast and deterministic direction math

#include <cmath>
#include <algorithm>
#include <stdint.h>

// -- Types --

using direction = uint16_t;

// -- Functions --

/// Sine with degrees represented as the full range of unsigned 16 bit. See https://www.nullhardware.com/blog/fixed-point-sine-and-cosine-for-embedded-systems/
static inline int32_t isin(direction val)
{
	const int16_t i = val;
	const uint32_t ii = (((i == (i | 0x4000)) ? (1 << 15) - i : i) & 0x7FFF) >> 1;
	const uint32_t y = ((ii * ((3370945099UL - ((ii * (ii * ((2746362156UL - ((ii * ((292421UL * ii) >> 13)) >> 3)) >> 13) >> 13)) >> 1)) >> 13)) + (1UL << 18)) >> 19;
	return (i < 0) ? -y : y;
}

/// Cosine with degrees represented as the full range of unsigned 16 bit.
static inline int32_t icos(direction val) { return isin(val + 16384U); }

/// Snap direction to nearest axis-aligned direction
static inline direction snap(direction dir) { return (dir + 0x2000) & 0xC000; }

static inline int isinr(direction dir, int dist) { return ((int64_t)dist * isin(dir)) / 65536; }
static inline int icosr(direction dir, int dist) { return ((int64_t)dist * icos(dir)) / 65536; }
