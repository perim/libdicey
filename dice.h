#pragma once

#include <stdint.h>
#include <vector>
#include <map>

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

struct roll_table
{
	int size = 0; // sum of all weights
	std::map<int, int> table; // sorted table of weights to index value
	inline int lookup(int key) const { return (*table.upper_bound(key)).second; }
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
	seed derive(uint64_t x) const { uint64_t r = orig ^ (fibonacci * x); return seed(r, r); }
	seed derive(uint64_t x, uint64_t y) const { uint64_t r = orig ^ (fibonacci * x * y); return seed(r, r); }
	seed derive(uint64_t x, uint64_t y, uint64_t z) const { uint64_t r = orig ^ (fibonacci * x * y * z); return seed(r, r); }
	seed derive(uint64_t x, uint64_t y, uint64_t z, uint64_t w) const { uint64_t r = orig ^ (fibonacci * x * y * z * w); return seed(r, r); }

	/// More advanced version of the above, including luck type and jackpot possiblity (eg a critical hit).
	int roll(int low, int high, luck_type luck, int jackpot_chance = 0, int jackpot_low = 0, int jackpot_high = 0, luck_type jackpot_luck = luck_type::normal);

	/// Multiple pseudo-random rolls against a roll table. If more than one, the rolls are guaranteed to be unique. Returns as many results as it could generate.
	/// 'roll_weight' increases chances of getting lower-weighted results. Is 0...128 but if accuracy is not important can use it as a percentage.
	/// 'start_index' tells function that 'results' already contains values that should not be duplicated.
	int unique_rolls(const roll_table* table, int count, int* results, luck_type rollee = luck_type::normal, int roll_weight = 0, int start_index = 0);

	/// Pseudo-random roll against a roll table. 'rw' is roll_weight as above.
	int roll(const roll_table* table, luck_type rollee = luck_type::normal, int rw = 0) { rw = (table->size * rw) >> 7; return table->lookup(roll(rw, table->size - rw, rollee)); }

	/// Multiple pseudo-random rolls against a roll table. Can return duplicate results. Returns as many results as it generates.
	int rolls(const roll_table* table, int count, int* results, luck_type rollee = luck_type::normal, int roll_weight = 0);

	/// Does a single roll on a roll table, modifying it by removing the entry hit. Returns -1 if table is empty. The next most rare entry in the roll table after the
	/// the deleted entry gains its chance to be rolled (so we do not have to rewrite the table).
	int boxgacha(roll_table* table, luck_type rollee = luck_type::normal, int roll_weight = 0);

	// Uniformly weighted rolls, where higher outcomes are less likely. Rolls are 0...high inclusive (or [0, high] in math terms).
	int pow2_weighted_roll(int high) { uint64_t n = 1 << (high + 2); uint64_t r = roll(1 << 1, n - 1); return high - (highestbitset(r) - 1); } // each value twice as unlikely as the previous, up to 64
	int quadratic_weighted_roll(int high) { uint64_t n = (high+1) * (high+1); uint64_t r = roll(0, n - 1); return high - (isqrt(r)); } // following a quadratic curve

	/// Current state
	uint64_t state;
	/// Original state
	uint64_t orig;
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

/// Take a list of weights and generate a roll table.
roll_table* roll_table_make(const std::vector<int>& input);

/// Generate a new random seed (not pseudo-random, real random).
seed seed_random();

/// Generate a deep copy of a roll table.
static inline roll_table* roll_table_copy(const roll_table* table) { return new roll_table(*table); }

/// Free a roll table.
static inline void roll_table_free(const roll_table* rt) { delete rt; }
