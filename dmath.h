#pragma once

// Fast and deterministic integer math

#include <cmath>
#include <algorithm>
#include <stdint.h>
#include <time.h>
#include <array>

// -- Constants --

constexpr uint64_t fibonacci = 11400714819323198485llu;

// -- Macros --

#define dicey_unlikely(expr) __builtin_expect((expr), 0)
#define dicey_likely(expr) __builtin_expect((expr), 1)

// -- Functions --

/// Return the index of the highest bit set.
#if __GNUC__
__attribute__((const)) static inline constexpr int highestbitset(uint64_t v) { return 64 - __builtin_clzll(v | 1) - 1; }
#else
__attribute__((const)) static inline constexpr int highestbitset(uint64_t v) { uint64_t r = 0; while (v >>= 1) r++; return r; }
#endif

/// See https://codereview.stackexchange.com/questions/69069/computing-the-square-root-of-a-64-bit-integer
__attribute__((const)) static inline constexpr uint64_t isqrt(uint64_t x) { uint64_t r = (uint64_t)sqrt(x); r = std::min(r, (uint64_t)UINT32_MAX); while (r * r > x) r--; while (x - r * r > r * 2) r++; return r; }

/// For 32bits we have enough precision
__attribute__((const)) static inline constexpr uint32_t isqrt32(uint32_t x) { return sqrt((double)x); }

/// See http://lemire.me/blog/2016/06/27/a-fast-alternative-to-the-modulo-reduction for an explanation of the snap to range magic.
__attribute__((const)) static inline constexpr uint64_t fastrange(uint64_t state, uint64_t low, uint64_t high) { return (uint64_t)(((__uint128_t)state * (__uint128_t)(high + 1 - low)) >> 64) + low; }

/// Fast check if an unsigned number is a power of two
__attribute__((const)) static inline constexpr bool ispow2(uint64_t x) { return x && !(x & (x - 1)); }

/// Your basic xorshift PRNG. State must be non-zero.
static inline constexpr uint64_t xorshift64(uint64_t& state) { uint64_t x = state; x ^= x << 13; x ^= x >> 7; x ^= x << 17; return state = x; }

/// Generic xorshift for any bitlength, assuming you have the right values to put into the equation. State must be non-zero.
static inline constexpr uint32_t xorshift_args(uint64_t& x, uint32_t a, uint32_t b, uint32_t c, uint32_t d, uint32_t e, uint32_t f) { x |= (x == 0); x ^= (x & a) << b; x ^= x >> c; x ^= (x & d) << e; return x & f; }

/// The splitmix64 PRNG used for value initialization, not changing state. See https://prng.di.unimi.it/splitmix64.c
__attribute__((const)) static inline constexpr uint64_t splitmix64(uint64_t state) { uint64_t z = (state += 0x9e3779b97f4a7c15); z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9; z = (z ^ (z >> 27)) * 0x94d049bb133111eb; return z ^ (z >> 31); }

/// Quick test if two ranges overlap.
__attribute__((const)) static inline constexpr bool range_overlap(int x1, int x2, int y1, int y2) { return x1 <= y2 && y1 <= x2; }

/// Turn any number into an even number
template<typename T> __attribute__((const)) static inline constexpr T make_even(T x) { return x & ~1; }
/// Turn any number into an odd number
template<typename T> __attribute__((const)) static inline constexpr T make_odd(T x) { return x & 1; }

/// Find smallest power-of-two number larger than or equal to x, where x must be larger than 0.
#if __GNUC__
__attribute__((const)) static inline uint32_t next_pow2(uint32_t x) { return 1 << (32 - __builtin_clz((x - 1) | 1)); }
#else
__attribute__((const)) static inline uint32_t next_pow2(uint32_t v) { v; v--; v |= v >> 1; v |= v >> 2; v |= v >> 4; v |= v >> 8; v |= v >> 16; v++; return v; }
#endif

/// Get your magic LSFR tap constant for a given bit width.
uint32_t lfsr_tap(uint32_t size) __attribute__((const));

/// Flexible linear feedback shift register. Get the magic tap constant for your bit length with lsfr_tap(). Your value will be stored in 'state'.
static inline constexpr void lfsr_next(uint64_t& state, uint32_t tap) { const uint_fast32_t lsb = state & 1; state >>= 1; state ^= (-lsb) & tap; }

/// Generate valid LFSR input state. Call lfsr_next() to get your first value.
static inline constexpr __attribute__((const))  uint64_t lfsr_init(uint64_t state, uint32_t bits) { return fastrange(splitmix64(state), 1, 1 << bits); }

/// CPU time measurement
static inline uint64_t cpu_gettime() { struct timespec t; clock_gettime(CLOCK_THREAD_CPUTIME_ID, &t); return ((uint64_t)t.tv_sec * 1000000000ull + (uint64_t)t.tv_nsec); }
