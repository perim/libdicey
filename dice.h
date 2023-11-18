#pragma once

#include <stdint.h>
#include <vector>
#include <map>
#include <numeric>

#include "dmath.h"

enum class luck_type
{
	normal,
	lucky,		// best out of two (lucky+unlucky=normal)
	unlucky,	// worst out of two (lucky+lucky=very lucky)
	very_lucky,	// best out of three
	very_unlucky,	// worst out of three
	mediocre,	// most average of min/max of two rolls
	uncommon,	// furthest from average of min/max of two
};

struct seed
{
	/// Generates a random number between 'low' and 'high', inclusive. Modifies the current random seed. Only positive numbers will work.
	int roll(int low, int high) { return fastrange(xorshift64(state), low, high); }

	/// This would almost always be a mistake, and could initialize the state to zero, which would be non-recoverable.
	seed() = delete;

	/// Initializing it to some value. Since we do not trust it to be a random set of bits, we hash it with splitmix64 which can handle zero inputs.
	explicit seed(uint64_t _state) : state(splitmix64(_state)), orig(state) {}

	/// If, for some reason, you want to save/restore seed state.
	seed(uint64_t _state, uint64_t _orig) : state(_state), orig(_orig) {}

	// Deriving new seeds with parameters, based on the original seed. This does not modify the original random seed.
	seed derive(uint64_t x) const { uint64_t r = orig ^ (fibonacci * splitmix64(x)); return seed(r, r); }
	seed derive(uint64_t x, uint64_t y) const { uint64_t r = orig ^ (fibonacci * splitmix64(x) * splitmix64(x + y)); return seed(r, r); }
	seed derive(uint64_t x, uint64_t y, uint64_t z) const { uint64_t r = orig ^ (fibonacci * splitmix64(x) * splitmix64(x + y) * splitmix64(x + y + z)); return seed(r, r); }
	seed derive(uint64_t x, uint64_t y, uint64_t z, uint64_t w) const { uint64_t r = orig ^ (fibonacci * splitmix64(x) * splitmix64(x + y) * splitmix64(x + y + z) * splitmix64(x + y + z + w)); return seed(r, r); }

	/// More advanced version of the above, including luck type and jackpot possiblity (eg a critical hit).
	int roll(int low, int high, luck_type luck, int jackpot_chance = 0, int jackpot_low = 0, int jackpot_high = 0, luck_type jackpot_luck = luck_type::normal);

	// Uniformly weighted rolls, where higher outcomes are less likely. Rolls are 0...high inclusive (or [0, high] in math terms).
	int pow2_weighted_roll(int high) { uint64_t n = 1 << (high + 2); uint64_t r = roll(1 << 1, n - 1); return high - (highestbitset(r) - 1); } // each value twice as unlikely as the previous, up to 64
	int quadratic_weighted_roll(int high) { uint64_t n = (high+1) * (high+1); uint64_t r = roll(0, n - 1); return high - (isqrt(r)); } // following a quadratic curve

	/// Shuffle any container that supports the size() member and [] operator.
	template<typename T> void shuffle(T& v) { for (size_t i = v.size() - 1; i > 0; --i) std::swap(v[i], v[roll(0, i + 1)]); }

	/// Current state
	uint64_t state;
	/// Original state
	uint64_t orig;
};

struct roll_table
{
	seed s;
	int size = 0; // sum of all weights
	std::map<int, int> table; // sorted table of weights to index value
	inline int lookup(int key) const { return (*table.upper_bound(key)).second; }

	/// Take a list of weights and generate a roll table.
	roll_table(const seed& orig, const std::vector<int>& input);

	/// Multiple pseudo-random rolls against a roll table. If more than one, the rolls are guaranteed to be unique. Returns as many results as it could generate.
	/// 'roll_weight' increases chances of getting lower-weighted results. Is 0...128 but if accuracy is not important can use it as a percentage.
	/// 'start_index' tells function that 'results' already contains values that should not be duplicated.
	int unique_rolls(int count, int* results, luck_type rollee = luck_type::normal, int roll_weight = 0, int start_index = 0);

	/// Pseudo-random roll against a roll table. 'rw' is roll_weight as above.
	int roll(luck_type rollee = luck_type::normal, int rw = 0) { rw = (size * rw) >> 7; return lookup(s.roll(rw, size - rw, rollee)); }

	/// Multiple pseudo-random rolls against a roll table. Can return duplicate results. Returns as many results as it generates.
	int rolls(int count, int* results, luck_type rollee = luck_type::normal, int roll_weight = 0);

	/// Does a single roll on a roll table, modifying it by removing the entry hit. Returns -1 if table is empty. The next most rare entry in the roll table after the
	/// the deleted entry gains its chance to be rolled (so we do not have to rewrite the table).
	int boxgacha(luck_type rollee = luck_type::normal, int roll_weight = 0);
};

struct const_roll_table
{
	const_roll_table(const std::vector<int>& weights);
	const_roll_table() = delete;

	int roll(seed& s) const
	{
		const int i = s.roll(0, size - 1);
		const int j = s.roll(0, sum);
		return (j < probability.at(i)) ? i : alias.at(i);
	}

	int size;
	std::vector<int> alias;
	std::vector<int> probability;
	long sum;
};

/// A simple and fast roll table that works like a deck of cards with equal probability on all options. It allows you to roll (draw), reset (shuffle), permanently remove the
/// previously rolled entry, and, if you define a range of extra entries, add new entries to the currently available ones. Roll, reset and remove are O(1) complexity, no matter
/// the size of the table. Construction and add are O(N). There are three different options for what automatically happens when the table is emptied: Reset, return zero or
/// return minus one. When adding a new entry, you need to reset the table first to get access to it.
enum class empty_table_policy
{
	reset,		// resets the table when emptied
	repeat_first,	// keeps returning index zero when emptied
	return_minus_one, // keeps returning minus one when emptied
};
struct linear_roll_table
{
	seed s;
	unsigned restricted = 0; // index of highest available entry
	int unused = 0; // index of highest unused entry
	unsigned removed = 0; // index of first removed entry; removed entries are above restricted ones
	std::vector<uint16_t> table;
	empty_table_policy policy;

	void reset() { unused = (int)restricted - 1; }
	int roll() { if (unused == -1) { if (policy == empty_table_policy::reset && restricted > 0) reset(); else return (policy == empty_table_policy::repeat_first) ? 0 : -1; } unsigned r = s.roll(0, unused); std::swap(table.at(r), table.at(unused)); unused--; return table[unused + 1]; }
	inline uint32_t size() const { return restricted; }
	inline uint32_t reserved() const { return table.size(); }
	inline uint32_t remaining() const { return unused + 1; }

	void remove() { if (restricted > 0) { removed--; std::swap(table.at(unused + 1), table.at(removed)); restricted--; } }
	bool add(unsigned idx)
	{
		const unsigned orig = idx;
		if (idx < restricted) for (idx = restricted; idx < removed && table.at(idx) != orig; idx++) ;
		if (idx >= restricted && idx < removed && table.at(idx) == orig) { if (idx > restricted) { std::swap(table.at(restricted), table.at(idx)); } restricted++; return true; }
		else return false;
	}

	linear_roll_table(const seed& orig, int entries, empty_table_policy _policy = empty_table_policy::reset, unsigned _restricted = 0) : s(orig), restricted(_restricted != 0 ? _restricted : entries), unused(restricted - 1), removed(entries), table(entries), policy(_policy) { std::iota(std::begin(table), std::end(table), 0); }
};

/// Another linear roll table. Unlike the above version, it does not store the whole table in memory and its construction is also O(1). It does not support add/remove of any one entry,
/// though, just an increase and decrease of the range of values. It can also be used more generally to construct pseudo-random sequences of non-repeating values without having to spend
/// memory on storing the whole series. Every operation is O(1) if the table size is power of two - 1, if not you may get hash collisions with probabiliy depending on the numerical distance
/// to the next power of two size. The restricted() method allows you to limit the used part of the range so you only get values below the given value. Note that the lower this limit is
/// compared to the maximum, the more collisions you will get and the slower next() will become. Also note that if you add new possible values with a higher value to restricted(), these
/// might be accessible only after a call to reset().
struct linear_series
{
	linear_series(const seed& orig, unsigned entries) : state(orig.state), cur(entries), unused(entries) { if (ispow2(entries)) entries++; len = next_pow2(entries); bits = highestbitset(len); tap = lfsr_tap(bits); x = lfsr_init(orig.state, bits); }

	void reset() { state = splitmix64(state); x = lfsr_init(state, bits); unused = cur; }
	uint32_t roll() { uint32_t ret; do { lfsr_next(x, tap); ret = x - 1; } while (ret >= cur); unused--; if (unused == 0) reset(); return ret; }
	inline uint32_t size() const { return cur; }
	inline uint32_t reserved() const { return len; }
	inline uint32_t remaining() const { return unused; }

	inline void restricted(uint32_t newsize) { cur = std::min(len, newsize); }

private:
	uint64_t state, x;
	uint32_t tap, len, cur, unused, bits;
};

/// Pseudo-random distribution (PRD) of a boolean chance, guaranteeing success no earlier than 50% before and no later than 50% after the average number of rolls. You use this if
/// you want something with a chance to happen, but want to smooth out bad luck from generating "unfun" sequences of results. Probability is in permille (1/1000). It is roughly
/// as fast as doing the random roll above, but you need to track more state.
enum class prd_function : uint8_t
{
	/// Entirely random but fair. Can have two unlikely results back-to-back.
	relaxed,
	/// A more strictly "fair" distribution, where a success only ever happens within 50% of the average outcome. You will not have two unlikely results back-to-back.
	fair,
	/// A distribution where only the starting point is random, after that it is only a matter of counting. When a good distribution is more important than not being able to predict or game it.
	predictable,
};
struct prd
{
	prd() = delete;
	// We need to invert the chance and count toward failures at above 50% probability of success.
	prd(const seed& orig, uint16_t probability, prd_function _func = prd_function::fair)
	  : state(orig.state), chance(probability < 500 ? probability : 1000 - probability), value(probability < 500 ? true : false), func(_func)
	{
		if (func == prd_function::predictable) { accum = fastrange(xorshift64(state), 0, 1000 / std::max<int>(chance, 1) - 1); }
		else reset();
	}
	inline bool roll() { if (accum) { accum--; return !value; } else { reset(); return value; } }
	/// A roll that accepts luck types normal, lucky and very lucky (and only those).
	bool roll(luck_type rollee) { if (rollee == luck_type::very_lucky && accum) accum--; if ((rollee == luck_type::lucky || rollee == luck_type::very_lucky) && accum) accum--; return roll(); }
	void reset()
	{
		if (func == prd_function::predictable) { accum = 1000 / std::max<int>(chance, 1) - 1; return; }
		const int max = (1000 / std::max<int>(chance, 1)) - 1;
		const int incr = (func == prd_function::fair) ? (max >> 1) : 0;
		const int high = max + remainder + incr - 1;
		const int low = incr + remainder;
		accum = fastrange(xorshift64(state), low, high) + 1;
		remainder = max - ((int)accum) + remainder;
	}
	// We maintain a total of 128 bits, or 16 bytes, of data.
	uint64_t state;
	uint16_t chance;
	uint16_t accum = 0;
	int16_t remainder = 0;
	uint8_t value;
	prd_function func; // 8bit
};

/// Merge two opposed luck types together, eg where you have a luck to hit and enemy has a luck not to be hit
luck_type luck_combine(luck_type rollee, luck_type against);

/// Generate a new random seed (not pseudo-random, real random).
seed seed_random();
