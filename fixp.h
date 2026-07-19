#pragma once

#include <cmath>
#include <algorithm>
#include <assert.h>
#include <stdint.h>

// -- Constants --

constexpr int64_t fraction_bits = 24;
constexpr int64_t integer_bits = 63 - fraction_bits;
constexpr int64_t fp_multiplier = 1 << fraction_bits;

// -- Types --

// Signed bitshifts are implementation defined until c++20 but it works as c++20 everywhere we care about.
struct fixp
{
	fixp(float x) : val(x * fp_multiplier) {}
	fixp(double x) : val(x * fp_multiplier) {}
	fixp(unsigned x) : val((int64_t)x << fraction_bits) {}
	fixp(uint64_t x) : val(x << fraction_bits) {}
	fixp(int x) : val((int64_t)x << fraction_bits) {}
	fixp(const fixp& x) : val(x.val) {}
	fixp() : val(0) {}

	inline float tofloat() const { return (double)val / (double)fp_multiplier; }
	inline double todouble() const { return (double)val / (double)fp_multiplier; }
	inline int toint() const { return val >> fraction_bits; }

	inline fixp& operator+=(const fixp& rhs) { val += rhs.val; return *this; }
	inline fixp& operator-=(const fixp& rhs) { val -= rhs.val; return *this; }
	inline fixp& operator*=(const fixp& rhs) { val *= rhs.val; val >>= fraction_bits; return *this; }
	inline fixp& operator/=(const fixp& rhs) { val = (val << fraction_bits) / rhs.val; return *this; }

	int64_t val;
};
inline fixp operator+(fixp lhs, const fixp& rhs) { lhs += rhs; return lhs; }
inline fixp operator-(fixp lhs, const fixp& rhs) { lhs -= rhs; return lhs; }
inline fixp operator-(const fixp& s) { return -s; }
inline fixp operator*(fixp lhs, const fixp& rhs) { lhs *= rhs; return lhs; }
inline fixp operator/(fixp lhs, const fixp& rhs) { lhs /= rhs; return lhs; }
inline bool operator<(fixp lhs, const fixp& rhs) { return lhs.val < rhs.val; } // the c++20 spaceship operator cannot come soon enough...
inline bool operator>(fixp lhs, const fixp& rhs) { return lhs.val > rhs.val; }
inline bool operator<=(fixp lhs, const fixp& rhs) { return lhs.val <= rhs.val; }
inline bool operator>=(fixp lhs, const fixp& rhs) { return lhs.val >= rhs.val; }
inline bool operator!=(fixp lhs, const fixp& rhs) { return lhs.val != rhs.val; }
inline bool operator==(fixp lhs, const fixp& rhs) { return lhs.val == rhs.val; }

struct ivec2
{
	fixp x;
	fixp y;
};

struct aabb
{
	ivec2 min;
	ivec2 max;

	ivec2 center() const { return ivec2{ (min.x + max.x) / 2, (min.y + max.y) / 2 }; }
};

// -- Functions --

static inline fixp fixp_sqrt(fixp x) { fixp r = sqrt(x.todouble()); r.val = std::min(r.val, (int64_t)INT64_MAX); while (r * r > x) r.val--; while (x - r * r > r * 2) r.val++; return r; }
static inline fixp fixp_pow(fixp base, unsigned exp) { fixp result = 1; for (;;) { if (exp & 1) result *= base; exp >>= 1; if (!exp) break; base *= base; } return result; }
static inline fixp fixp_intersect(fixp da, fixp db) { return da / (da - db); }
static inline fixp fixp_dot(ivec2 a, ivec2 b) { return a.x * b.x + a.y * b.y; }
static inline fixp fixp_distance(ivec2 a, ivec2 b) { fixp dx = a.x - b.x; fixp dy = a.y - b.y; return fixp_sqrt(dx * dx + dy * dy); }
static inline fixp fixp_length(ivec2 a) { return fixp_sqrt(fixp_dot(a, a)); }
static inline ivec2 fixp_normal(ivec2 a) { const fixp len = fixp_length(a); ivec2 r; r.x = a.x / len; r.y = a.y / len; return r; }
static inline bool fixp_overlaps(aabb a, aabb b) { int d0 = b.max.x < a.min.x; int d1 = a.max.x < b.min.x; int d2 = b.max.y < a.min.y; int d3 = a.max.y < b.min.y; return !(d0 | d1 | d2 | d3); }

// asin approximation via A&S 4.4.57 (Estrin form), valid for x in [-1, 1], max error ~7e-5
// Adapted from https://16bpp.net/blog/post/even-faster-asin-was-staring-right-at-me/
static inline fixp fixp_asin(fixp x)
{
	constexpr int64_t c_halfpi = (int64_t)(1.5707963267948966 * fp_multiplier);
	constexpr int64_t c_a0 = (int64_t)(1.5707288 * fp_multiplier);
	constexpr int64_t c_a1 = (int64_t)(-0.2121144 * fp_multiplier);
	constexpr int64_t c_a2 = (int64_t)(0.0742610 * fp_multiplier);
	constexpr int64_t c_a3 = (int64_t)(-0.0187293 * fp_multiplier);

	fixp abs_x; abs_x.val = x.val < 0 ? -x.val : x.val;
	const fixp x2 = abs_x * abs_x;
	fixp a0; a0.val = c_a0; fixp a1; a1.val = c_a1; fixp a2; a2.val = c_a2; fixp a3; a3.val = c_a3;
	const fixp p = (a3 * abs_x + a2) * x2 + (a1 * abs_x + a0);
	fixp one_minus_x; one_minus_x.val = (int64_t)fp_multiplier - abs_x.val;
	const fixp x_diff = fixp_sqrt(one_minus_x);
	fixp result; result.val = c_halfpi - (x_diff * p).val;
	if (x.val < 0) result.val = -result.val;
	return result;
}
