#pragma once

// Fast and deterministic integer math

#include <cmath>
#include <algorithm>
#include <stdint.h>

// -- Constants --

constexpr uint64_t fibonacci = 11400714819323198485llu;

// -- Functions --

/// Return highest bit set. We could also use a compiler intrinsic, eg { return __builtin_clzll(v); } on gcc-compatible compilers.
static inline int highestbitset(uint64_t v) { uint64_t r = 0; while (v >>= 1) r++; return r; }

/// See https://codereview.stackexchange.com/questions/69069/computing-the-square-root-of-a-64-bit-integer
static inline uint64_t isqrt(uint64_t x) { uint64_t r = (uint64_t)sqrt(x); r = std::min(r, (uint64_t)UINT32_MAX); while (r * r > x) r--; while (x - r * r > r * 2) r++; return r; }

/// For 32bits we have enough precision
static inline uint32_t isqrt32(uint32_t x) { return sqrt((double)x); }

/// See http://lemire.me/blog/2016/06/27/a-fast-alternative-to-the-modulo-reduction for an explanation of the snap to range magic.
static inline uint64_t fastrange(uint64_t state, uint64_t low, uint64_t high) { return (uint64_t)(((__uint128_t)state * (__uint128_t)(high + 1 - low)) >> 64) + low; }

/// Fast check if an unsigned number is a power of two
static inline bool ispow2(uint64_t x) { return x && !(x & (x - 1)); }

/// Your basic xorshift PRNG. This can be improved in many ways at the cost of more CPU cycles but is usually good enough.
static inline uint64_t xorshift64(uint64_t& state) { uint64_t x = state; x ^= x << 13; x ^= x >> 7; x ^= x << 17; return state = x; }

/// Quick test if two ranges overlap.
static inline bool range_overlap(int x1, int x2, int y1, int y2) { return x1 <= y2 && y1 <= x2; }
