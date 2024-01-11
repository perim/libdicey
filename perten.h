#pragma once

// -- 2^10 fractional math --
// Floating point is notoriously non-deterministic when we want to be portable and can be slow, while numbers based around 100 (percentage) or any other
// 10-based value are not good for fast or accurate math. For this reason we here define a 2^10 system instead which we call "perten" values. Math using
// these values avoid most divisions and use less multiply instructions. These values are capped at their numerical maximum, and will never overflow or
// wrap around while using this API. The design is a bit clunky to make it so strongly typed that you cannot accidentally intermix it with other types
// of values. It is designed to easily and safely implement a multiplicative stats system.

// An "increase" approaches the type maximum, while a "reduce" approaches the base value ceiling. These can both be used on the same
// value if they are meant to be counteracting each other. Be careful about conversions back and forth between perten and percent, especially if you use
// percent as a intermediary value, as this conversion can be lossy and lead to cascading errors. For example percent -> perten -> percent is stable,
// while perten -> percent -> perten is not. For example adding a 10% value in perten and multiplying it by 10 will not yield you exactly 100%. We can
// safely operate on 0.1% precision and up to 4 million in total value, any precision beyond this point will be lost.

#include "dmath.h"
#include <cassert>

/// Ten bits precision
const int perten_bits = 10;

/// `perten_base` is equal to 1.0 or 100%
const uint32_t perten_base = (1 << perten_bits);

struct perten
{
	uint32_t value = 0;
	bool operator==(const perten& rhs) const { return value == rhs.value; }
	bool operator<=(const perten& rhs) const { return value <= rhs.value; }
	bool operator>=(const perten& rhs) const { return value >= rhs.value; }
	bool operator!=(const perten& rhs) const { return value != rhs.value; }
	bool operator>(const perten& rhs) const { return value > rhs.value; }
	bool operator<(const perten& rhs) const { return value < rhs.value; }
	perten operator+(const perten& rhs) const { return perten{value + rhs.value}; }
	perten operator-(const perten& rhs) const { return perten{value - rhs.value}; }
	void operator+=(const perten& rhs) { value += rhs.value; }
	void operator-=(const perten& rhs) { value -= rhs.value; }
/*
	perten& operator=(perten& other) { value = other; }
	perten operator=(perten other) { value = other; }
	perten operator=(uint32_t other) { value = other << perten_bits; }
	perten operator=(int other) { value = other << perten_bits; }*/
};

// Using these constants rather than converting from percentages preserves perfect accuracy
const perten perten_empty = perten{0};
const perten perten_full = perten{perten_base}; // 1024
const perten perten_half = perten{1 << (perten_bits - 1)}; // 512
const perten perten_quarter = perten{1 << (perten_bits - 2)}; // 256, or 25%
const perten perten_eight = perten{1 << (perten_bits - 3)}; // 128, close to 12%
const perten perten_16th = perten{1 << (perten_bits - 4)}; // 64, close to 6%
const perten perten_32th = perten{1 << (perten_bits - 5)}; // 32, close to 3%
const perten perten_64th = perten{1 << (perten_bits - 6)}; // 16, close to 1.6%
const perten perten_128th = perten{1 << (perten_bits - 7)}; // 8, close to 0.8%
const perten perten_256th = perten{1 << (perten_bits - 8)}; // 4, close to 0.4%
const perten perten_512th = perten{1 << (perten_bits - 9)}; // 2, close to 0.2%
const perten perten_1024th = perten{1 << (perten_bits - 10)}; // 1, close to 0.1%
const perten perten_double = perten{perten_base * 2};
const perten perten_triple = perten{perten_base * 3};
const perten perten_tenth = perten{102}; // but note how perten_tenth * 10 != perten_full
const perten perten_max = perten{UINT32_MAX};

/// Reduce `source` by `amount` and reduce `amount` by the amount actually reduced. Cannot reduce below zero.
static inline void perten_drain(perten& amount, perten& source) { const uint32_t old_amount = amount.value; amount.value -= std::min<uint32_t>(amount.value, source.value); source.value -= std::min<uint32_t>(old_amount, source.value); }
static inline __attribute__((const)) perten perten_multiply(perten a, perten b) { return perten{(uint32_t)((uint64_t)a.value * (uint64_t)b.value) >> perten_bits}; }
static inline __attribute__((const)) int perten_to_int(perten v) { return (int)(v.value >> perten_bits); }
static inline __attribute__((const)) unsigned perten_to_uint(perten v) { return (v.value >> perten_bits); }
static inline __attribute__((const)) double perten_to_float(perten v) { return ((double)v.value / (double)perten_base); }
static inline __attribute__((const)) int perten_to_percent(perten v) { return (((uint64_t)v.value + 1) * 100u) >> perten_bits; }
static inline __attribute__((const)) double perten_to_percentf(perten v) { return ceil((double)v.value * 1000.0f / (double)perten_base) / 10.0f; }
static inline __attribute__((const)) perten perten_from_percent(int v) { return perten{(uint32_t)std::min<uint64_t>((((uint32_t)v) << perten_bits) / 100u, UINT32_MAX)}; }
static inline __attribute__((const)) perten perten_from_percent(uint32_t v) { return perten{(uint32_t)std::min<uint64_t>((v << perten_bits) / 100u, UINT32_MAX)}; }
static inline __attribute__((const)) perten perten_from_percent(double v) { return perten{(uint32_t)((v * perten_base) / 100.0f)}; }
static inline __attribute__((const)) perten perten_from_uint(uint32_t v) { return perten{v << perten_bits}; }
static inline __attribute__((const)) perten perten_from_int(int v) { assert(v >= 0); return perten{((uint32_t)v) << perten_bits}; }
/// Multiplicatively increase a perten value by another perten value. It will not overflow.
static inline __attribute__((const)) perten perten_increase(perten orig, perten mod) { return perten{(uint32_t)std::min<uint64_t>(((uint64_t)orig.value * (mod.value + perten_base)) >> perten_bits, UINT32_MAX)}; }
/// Multiplicatively reduce a perten value by another perten value. It will approach but never reach the base value (ie 1.0 or 100%).
static inline __attribute__((const)) perten perten_reduce(perten orig, perten mod) { return perten{((uint32_t)((uint64_t)orig.value * (perten_base - mod.value)) >> perten_bits)}; }
/// Modify any value by a perten value.
static inline __attribute__((const)) perten perten_apply(perten orig, perten mod) { return perten{(uint32_t)std::min<uint64_t>(((uint64_t)orig.value * (uint64_t)mod.value) >> perten_bits, UINT32_MAX)}; }
/// Reverse a modification to any value by a perten value.
static inline __attribute__((const)) perten perten_apply_reverse(perten orig, perten mod) { return perten{(uint32_t)((uint64_t)orig.value << perten_bits) / mod.value}; }

// -- Lazy conditional perten multiplicative matrix
// This implements a simple cache for a matrix of values, where each column has a conditional and the only output is the cached sum
// of the column values for a given row. The type has to be unsigned. The maximum value of T is reserved for cache maintenance.
template<size_t C, size_t R>
struct lazy_conditional_matrix
{
	lazy_conditional_matrix() { enabled.fill(false); cache.fill(perten_max); for (auto& i : matrix) for (auto& j : i) j = perten_full; }

	/// return multiplicative sum of all enabled columns
	inline perten row(int r) { if (cache[r] == perten_max) calc(r); return cache[r]; }

	/// modify one value and dirty its cache line
	inline void modify(int c, int r, perten value) { matrix[c][r] = value; cache[r] = perten_max; }

	/// modify one condition and dirty relevant cache lines
	void toggle(int c, bool v) { if (enabled[c] != v) { enabled[c] = v; for (unsigned r = 0; r < R; r++) if (matrix[c][r] != perten_full) cache[r] = perten_max; } }

	inline bool is_enabled(int c) const { return enabled[c]; }

private:
	void calc(int r) { cache[r] = perten_full; for (unsigned c = 0; c < C; c++) { if (enabled[c]) cache[r] = perten_multiply(cache[r], matrix[c][r]); } }
	std::array<std::array<perten, R>, C> matrix;
	std::array<uint_fast8_t, C> enabled;
	std::array<perten, R> cache;
};
