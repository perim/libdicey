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
static inline constexpr void lfsr_next(uint64_t& state, uint32_t tap) { const uint_fast32_t lsb = state & 1; state >>= 1; if (lsb) state ^= tap; }

/// Generate valid LFSR input state. Call lfsr_next() to get your first value.
static inline constexpr __attribute__((const))  uint64_t lfsr_init(uint64_t state, uint32_t bits) { return fastrange(splitmix64(state), 1, 1 << bits); }

/// CPU time measurement
static inline uint64_t cpu_gettime() { struct timespec t; clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t); return ((uint64_t)t.tv_sec * 1000000000ull + (uint64_t)t.tv_nsec); }

// -- 2^10 fractional math --
// Numbers based around 100 are not good to do fast math on, so we can use to a 2^10 system instead. We call these "perten" values. Math using
// these values avoid most divisions and use less multiplies. These values are capped at their numerical maximum, and will never overflow while
// using this API. An "increase" goes to infinity, while a "reduction" approaches but never reaches the base value ceiling. These can both be
// used on the same value if they are meant to be counteracting each other. Be careful about conversions back and forth between perten and
// percent, especially if you use percent as a intermediary value, as this conversion can be lossy and lead to cascading errors. For example
// percent -> perten -> percent is stable, while perten -> percent -> perten is not. We can safely operate on down to 0.1% values, but floating
// point precision beyond this point will be lost.
using perten = uint32_t;
const int perten_bits = 10;
const perten perten_base = (1 << perten_bits);
static inline constexpr __attribute__((const)) int perten_to_percent(perten v) { return (((uint64_t)v + 1) * 100u) >> perten_bits; }
static inline constexpr __attribute__((const)) float perten_to_percentf(perten v) { return ceil((float)v * 1000.0f / (float)perten_base) / 10.0f; }
static inline constexpr __attribute__((const)) perten percent_to_perten(int v) { return std::min<uint64_t>((v << perten_bits) / 100u, UINT32_MAX); }
static inline constexpr __attribute__((const)) perten percent_to_perten(float v) { return (v * perten_base) / 100.0f; }
static inline constexpr __attribute__((const)) perten perten_add_increase(perten orig, perten mod) { return std::min<uint64_t>(((uint64_t)orig * (mod + perten_base)) >> perten_bits, UINT32_MAX); }
static inline constexpr __attribute__((const)) perten perten_add_reduction(perten orig, perten mod) { return ((uint64_t)orig * (perten_base - mod)) >> perten_bits; }
static inline constexpr __attribute__((const)) perten perten_apply(perten orig, perten mod) { return std::min<uint64_t>(((uint64_t)orig * (uint64_t)mod) >> perten_bits, UINT32_MAX); }
static inline constexpr __attribute__((const)) perten perten_apply_reverse(perten orig, perten mod) { return ((uint64_t)orig << perten_bits) / mod; }

// -- Lazy conditional perten multiplicative matrix
// This implements a simple cache for a matrix of values, where each column has a conditional and the only output is the cached sum
// of the column values for a given row. The type has to be unsigned. The maximum value of T is reserved for cache maintenance.
template<size_t C, size_t R>
struct lazy_conditional_matrix
{
	lazy_conditional_matrix() { enabled.fill(false); cache.fill(UINT32_MAX); for (auto& i : matrix) for (auto& j : i) j = perten_base; }

	/// return multiplicative sum of all enabled columns
	inline perten row(int r) { if (cache[r] == UINT32_MAX) calc(r); return cache[r]; }

	/// modify one value and dirty its cache line
	inline void modify(int c, int r, perten value) { matrix[c][r] = value; cache[r] = UINT32_MAX; }

	/// modify one condition and dirty relevant cache lines
	void toggle(int c, bool v) { if (enabled[c] != v) { enabled[c] = v; for (unsigned r = 0; r < R; r++) if (matrix[c][r] != perten_base) cache[r] = UINT32_MAX; } }

	inline bool is_enabled(int c) const { return enabled[c]; }

private:
	void calc(int r) { cache[r] = perten_base; for (unsigned c = 0; c < C; c++) { if (enabled[c]) cache[r] = (cache[r] * matrix[c][r]) >> perten_bits; } }
	std::array<std::array<perten, R>, C> matrix;
	std::array<uint_fast8_t, C> enabled;
	std::array<perten, R> cache;
};
