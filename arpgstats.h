#pragma once

#include <cassert>
#include <array>
#include <cstdint>
#include <vector>
#include <forward_list>
#include <algorithm>

#include "dmath.h"

#include <stdio.h>

namespace arpg
{

/// Game-wide setup state
const int power_types = 3;
const int damage_types = 5;
const int skill_types = 3; // eg spell, attack, debuff spell
const int skill_slots = 5;
const int pop_ticks = 2; // how long skills off cooldown should "pop" visually
const uint32_t max_effect_secs = 10;
const int game_statuses = 20;
const int base_excess_loss_rate = 8;
const int highres_ticks = 0; // additional subsecond ticks to run
const int damage_dealt_as_dot_seconds = 4;

enum status_per_power
{
	power_maximum,
	power_excess_loss_rate,
	power_instant_damage_taken,
	power_dot_taken,
	power_recovery_modify, // modify amount received each tick
	power_recovery_time, // modify time gained which means more total recovery in the end
	power_regeneration_rate,
	power_cost_modify,
	statuses_per_power_type
};
enum status_per_damage
{
	damage_instant_damage,
	damage_dot,
	damage_dot_exponential, // each second applied gets modified
	damage_dot_time_modify, // modify the time the damage is applied to, changing total amount
	damage_distribution_modify, // change distribution factors between powers
	statuses_per_damage_type
};
enum status_per_skill
{
	windup_time_modifier, // "casting speed", time stuck in interruptible animation before doing skill
	animation_time_modifier, // "attack speed", time stuck in animation while doing skill (before/after)
	cooldown_time_modifier, // time before skill is available for use again, higher is shorter cooldown
	interrupt_cooldown_modifier, // modify the cooldown if interrupted, less is shorter cooldown
	statuses_per_skill_type,
};
using damage_mods = std::array<uint16_t, statuses_per_damage_type>;
using damage_mods_full = std::array<damage_mods, damage_types>;
using damage_also_dealt_as = std::array<perten, damage_types>; // instant and dot

const int first_status_per_damage_taken_type = 0;
const int first_status_per_power_type = statuses_per_damage_type * damage_types;
const int first_status_damage_distribution = first_status_per_power_type + statuses_per_power_type * power_types;
const int first_status_per_damage_dealt_type = first_status_damage_distribution + ((power_types - 1) * damage_types);
const int first_game_status = first_status_per_damage_dealt_type + statuses_per_damage_type * damage_types;
const int max_statuses = first_game_status + game_statuses;

inline int constexpr damage_distribution_index(int damage_type, int power_type) { return first_status_damage_distribution + damage_type + power_type * damage_types; }
inline int constexpr damage_status_index(status_per_damage e, int damage_type) { return first_status_per_damage_taken_type + statuses_per_damage_type * damage_type + e; }
inline int constexpr power_status_index(status_per_power e, int power_type) { return first_status_per_power_type + statuses_per_power_type * power_type + e; }

static inline void take(int& amount, uint16_t& source)
{
	const int old_amount = amount;
	amount -= std::min<int>(amount, source);
	source -= std::min<int>(old_amount, source);
}

struct power_def
{
	uint16_t current = perten_base;
	uint16_t enhanced = 0;
	uint16_t excess = 0;
	std::array<uint16_t, max_effect_secs> recovery; // see below

	inline int reduce(int share)
	{
		take(share, excess);
		take(share, enhanced);
		take(share, current);
		return share; // if non-zero, we did not have enough, return deficit
	}
};

struct damage_def
{
	/// We can eat out of the DoT second as fine-grained as we want. When we apply a DoT, we apply it
	/// to as many seconds into the future as required, starting at the current_dot_second and rotating
	/// around if necessary. DoTs cannot be added with time in fraction of a second.
	std::array<uint16_t, max_effect_secs> dot;
};

struct timed_status_def
{
	int16_t time;
	uint16_t index;
	uint16_t value; // time in 1/1000th ticks
};

enum skill_states
{
	skill_state_ready,
	skill_state_windup,
	skill_state_windup_done, // higher level code pick me up now
	skill_state_animation,
	skill_state_cooldown,
};

struct skill_slot_def
{
	int8_t skill = -1; // -1 = no skill in slot
	uint8_t cost_power_type = 0;
	uint8_t state = skill_state_ready;
	uint8_t flags = 0; // ?? if network time is minimum for animation time + is channeled skill
	uint16_t cost_power_amount = 0;
	uint16_t value = 0; // state's countdown value in 1/1000th ticks; _can_ be higher than its max below due to buffs/debuffs or status effects
	uint16_t windup_time = 0;
	uint16_t animation_time = 0;
	uint16_t cooldown_time = 0; // in ticks
};

struct entity
{
	entity()
	{
		statuses.fill(perten_base);
		for (unsigned i = 0; i < power_types; i++) statuses[power_status_index(power_excess_loss_rate, i)] = perten_base - base_excess_loss_rate;
		for (unsigned i = 0; i < power_types; i++) powers[i].recovery.fill(0);
		for (unsigned i = 0; i < damage_types; i++) damage[i].dot.fill(0);
		for (unsigned i = 0; i < power_types; i++) for (unsigned j = 0; j < damage_types; j++) statuses[damage_distribution_index(j, i)] = 0;
		statuses_original = statuses;
		for (unsigned j = 0; j < damage_types; j++) statuses[damage_distribution_index(j, 0)] = perten_base; // the backup values for these are special
	}

	uint8_t active = true;
	uint8_t highres = true; // entity uses high-resolution timed statuses and powers
	std::array<power_def, power_types> powers;
	std::array<damage_def, damage_types> damage;
	std::array<skill_slot_def, skill_slots> slots;

	std::array<uint16_t, max_statuses> statuses;
	std::array<uint16_t, max_statuses> statuses_original;
	std::forward_list<timed_status_def> timed_statuses;

	inline uint16_t damage_distribution(int damage_type, int power_type) { return statuses[damage_distribution_index(damage_type, power_type)]; }
	inline uint16_t damage_status(status_per_damage e, int damage_type) { return statuses[damage_status_index(e, damage_type)]; }
	inline uint16_t power_status(status_per_power e, int power_type) { return statuses[power_status_index(e, power_type)]; }

	// --- Skills ---

	/// Pay casting cost for a spell or the maintenance cost of a channeled spell, or return false
	bool try_pay_skill(int slot) { skill_slot_def& s = slots[slot]; return try_expense(s.cost_power_type, s.cost_power_amount); }

	/// Initiate a skill action
	bool try_start_skill(int slot)
	{
		bool r = try_pay_skill(slot);
		if (r)
		{
			skill_slot_def& s = slots[slot];
			if (s.windup_time > 0)
			{
				s.state = skill_state_windup;
				s.value = s.windup_time << perten_bits;
			} else {
				s.state = skill_state_animation;
				s.value = s.animation_time << perten_bits;
			}
		}
		return r;
	}

	/// Interrupt skill
	bool try_interrupt_skill(int slot)
	{
		skill_slot_def& s = slots[slot];
		if (s.state != skill_state_windup) return false;
		s.state = skill_state_cooldown;
		s.value = perten_apply(interrupt_cooldown_modifier, s.cooldown_time) << perten_bits;
		return true;
	}

	/// Is skill ready to be used?
	bool skill_ready(int slot) const { const skill_slot_def& s = slots[slot]; return s.state == skill_state_ready; }  // TBD check power cost?

	/// You can pop the skill visualization right after the skill becomes available again after a cooldown. Value between 0.0 (nothing) and 1.0 (max).
	/// The f value is the fraction of rendering time since the last time until the next tick from 0.0 to 1.0.
	double skill_pop_visualized(int slot, double f) const { const skill_slot_def& s = slots[slot]; if (s.state != skill_state_ready || s.value == 0) return 0.0; else return (1.0 / pop_ticks) * ((double)s.value - f); }

	/// How much cooldown to visualize on a skill slot from zero (none) to one (all). The f value is the fraction of rendering time since the last time until the next tick from 0.0 to 1.0.
	double skill_cooldown_visualized(int slot, double f) const { const skill_slot_def& s = slots[slot]; if (s.state != skill_state_cooldown || s.value == 0) return 0.0; else return ceil((100.0 / (double)s.cooldown_time) * std::min<double>(s.cooldown_time, (double)s.value - f)) / 100.0; }

	/// Current - what we have right now. Recovering - the extra amount that is being recovered over time.
	/// Losing - the amount of the current that is being lost over time. TBD maybe cache these, also used in inactive_candidate()
	void power_bar(int power_type, uint32_t& maximum, uint32_t& current, uint32_t& recovering, uint32_t& losing)
	{
		maximum = power_status(power_maximum, power_type);
		current = powers[power_type].current;
		recovering = 0;
		losing = 0;
		for (unsigned i = 0; i < powers[i].recovery.size(); i++) recovering += powers[power_type].recovery[i];
		for (unsigned i = 0; i < damage[i].dot.size(); i++) losing += damage[power_type].dot[i]; // TBD apply damage_dot and power_dot_taken?
		recovering = std::min(recovering, maximum - current);
		losing = std::min(losing, current);
	}

	// --- Maintenance ---

	void cleanse() // remove all DoT and timed effects, excess power
	{
		for (power_def& p : powers)
		{
			p.excess = 0;
		}
		for (unsigned i = 0; i < power_types; i++) powers[i].recovery.fill(0);
		for (unsigned i = 0; i < damage_types; i++) damage[i].dot.fill(0);
		timed_statuses.clear();
		statuses = statuses_original;
	}

	void reset() // as above + reset all power data
	{
		cleanse();
		for (int power_type = 0; power_type < power_types; power_type++)
		{
			power_def& p = powers[power_type];
			p.current = power_status(power_maximum, power_type);
			p.enhanced = 0;
		}
	}

	inline bool inactive_candidate() const
	{
		for (const auto& v : timed_statuses) { (void)v; return false; }
		for (int power_type = 0; power_type < power_types; power_type++)
		{
			for (unsigned i = 0; i < powers[i].recovery.size(); i++) if (powers[power_type].recovery[i] > 0) return false;
			for (unsigned i = 0; i < damage[i].dot.size(); i++) if (damage[power_type].dot[i] > 0) return false;
		}
		return true;
	}

	// --- Statuses ---

	inline void status_update_and_reapply(int index)
	{
		statuses[index] = statuses_original[index]; // update current, discarding timed effects
		for (auto v : timed_statuses) // reapply timed effects
		{
			if (v.index == index) statuses[index] = perten_apply(statuses[index], v.value);
		}
	}
	inline void status_increase(int index, perten value)
	{
		statuses_original[index] = perten_add_increase(statuses_original[index], value);
		status_update_and_reapply(index);
	}
	inline void status_decrease(int index, int value)
	{
		statuses_original[index] = perten_add_reduction(statuses_original[index], value);
		status_update_and_reapply(index);
	}
	inline void timed_status_increase(uint16_t index, uint16_t value, int16_t secs)
	{
		statuses[index] = perten_add_increase(statuses[index], value);
		value += 100;
		timed_statuses.push_front({secs, index, value});
	}
	inline void timed_status_decrease(uint16_t index, uint16_t value, int16_t secs)
	{
		statuses[index] = perten_add_reduction(statuses[index], value);
		timed_statuses.push_front({secs, index, value});
	}

	// --- Tick ---

	void second_tick()
	{
		auto before = timed_statuses.before_begin();
		for (auto it = timed_statuses.begin(); it != timed_statuses.end(); )
		{
			if (it->time <= 0)
			{
				int index = it->index;
				it = timed_statuses.erase_after(before);
				status_update_and_reapply(index);
			}
			else
			{
				it->time--;
				before = it;
				++it;
			}
		}

		// Go through skill slots
		for (int i = 0; i < skill_slots; i++)
		{
			skill_slot_def& s = slots[i];
			if (s.value == 0) continue;
			if (s.state == skill_state_windup)
			{
				s.value = std::max(0, (int)s.value - (int)perten_apply(perten_base, windup_time_modifier));
				if (s.value == 0) s.state = skill_state_windup_done; // need to wait for higher level code here
			}
			else if (s.state == skill_state_animation)
			{
				s.value = std::max(0, (int)s.value - (int)perten_apply(perten_base, animation_time_modifier));
				if (s.value == 0)
				{
					s.state = skill_state_cooldown;
					s.value = s.cooldown_time << perten_bits;
				}
			}
			else if (s.state == skill_state_cooldown)
			{
				s.value = std::max(0, (int)s.value - (int)perten_apply(perten_base, cooldown_time_modifier));
				if (s.value == 0)
				{
					s.state = skill_state_ready;
					s.value = pop_ticks << perten_bits;
				}
			}
		}
	}

	// --- Damage / Power ---

	// note that damage amount is modified when ticked, not when applied, giving player time to apply countermeasures or debuffs
	void apply_damage_over_time(int current_effect_second, int damage_type, uint32_t seconds, uint32_t amount, const damage_mods& offense)
	{
		perten modify_time = perten_apply(damage_status(damage_dot_time_modify, damage_type), offense[damage_dot_time_modify]);
		perten exponential = perten_apply(damage_status(damage_dot_exponential, damage_type), offense[damage_dot_exponential]);
		assert(seconds <= max_effect_secs);
		seconds = std::min(max_effect_secs, perten_apply(seconds, modify_time));
		amount = perten_apply(amount, offense[damage_dot]);
		assert(damage_type < damage_types);
		while (seconds && amount)
		{
			damage[damage_type].dot[current_effect_second] += amount;
			amount = perten_apply(amount, exponential);
			seconds--;
			current_effect_second = (current_effect_second + 1) % max_effect_secs;
		}
	}

	void apply_recover_effect(int current_effect_second, int power_type, uint32_t seconds, uint32_t amount)
	{
		assert(seconds <= max_effect_secs);
		assert(power_type < power_types);
		seconds = std::min(max_effect_secs, perten_apply(seconds, power_status(power_recovery_time, power_type)));
		while (seconds && amount)
		{
			powers[power_type].recovery[current_effect_second] += amount;
			seconds--;
			current_effect_second = (current_effect_second + 1) % max_effect_secs;
		}
	}

	int apply_damage_full(int current_effect_second, int damage_type, int amount, const damage_mods_full& offense, const damage_also_dealt_as& instant, const damage_also_dealt_as& dot)
	{
		int r = apply_damage(damage_type, amount, offense[damage_type]); // native type
		apply_damage_over_time(current_effect_second, damage_type, damage_dealt_as_dot_seconds, perten_apply(amount, dot[damage_type]), offense[damage_type]);
		int type = (damage_type + 1) % damage_types;
		while (type != damage_type)
		{
			r += apply_damage(type, perten_apply(amount, instant[type]), offense[type]);
			apply_damage_over_time(current_effect_second, type, damage_dealt_as_dot_seconds, perten_apply(amount, dot[type]), offense[type]);
			type = (type + 1) % damage_types;
		}
		return r;
	}

	/// Attempt to spend a power type that costs a certain amount of it. If this amount is not available, do not expense anything and just return false.
	bool try_expense(int power_type, int amount)
	{
		const int cost = perten_apply(amount, power_status(power_cost_modify, power_type));
		power_def& p = powers[power_type];
		const int has = p.current + p.excess + p.enhanced;
		if (cost > has) return false;
		p.reduce(cost);
		return true;
	}

	int apply_damage(int damage_type, int amount, const damage_mods& offense)
	{
		perten modify = perten_apply(damage_status(damage_instant_damage, damage_type), offense[damage_instant_damage]);
		perten distribution_modify = perten_apply(damage_status(damage_distribution_modify, damage_type), offense[damage_distribution_modify]);
		amount = perten_apply(amount, modify);
		// we're assuming power zero is most important, ie health, so going in reverse order
		for (int power_type = (int)powers.size() - 1; power_type >= 0; power_type--)
		{
			power_def& p = powers[power_type];
			perten distribution = perten_apply(damage_distribution(damage_type, power_type), distribution_modify);
			int share = perten_apply(amount, distribution);
			amount -= share;
			share = perten_apply(share, power_status(power_instant_damage_taken, power_type)); // apply power-related damage mod
			amount += p.reduce(share); // roll leftover damage "back in" to dealt with by other powers
		}
		return amount; // if over zero, died - returns overkill damage; if less than zero, hit a damage threshold
	}
};

struct stats
{
	std::vector<entity> entities;

	/// You typically want to do subsecond ticks for player characters and bosses, or possibly all entities on screen, for smoother visuals.
	/// You must only ever do a fixed number of subsecond ticks per second that never changes. Recommended: 1 or 3. If you have eg 4 ticks per
	/// second, call this 3 times then second_tick() once. Also used to give timed status effects higher precision.
	void subsecond_tick()
	{
		for (entity& e : entities)
		{
			if (!e.active || !e.highres) continue;
			// TBD do partial update here
		}
	}

	void second_tick()
	{
		const int next_effect = (current_effect_second + 1) % max_effect_secs;
		for (entity& e : entities)
		{
			if (!e.active) continue;
			e.second_tick();
			// first recovery / healing
			for (unsigned i = 0; i < e.powers.size(); i++)
			{
				power_def& p = e.powers[i];
				unsigned maximum = e.power_status(power_maximum, i);
				p.current = std::min<int>(maximum, p.current + perten_apply(p.recovery[current_effect_second], e.power_status(power_recovery_modify, i)));
				p.recovery[current_effect_second] = 0;
				p.recovery[next_effect] += (perten_apply(maximum, e.power_status(power_regeneration_rate, i)) >> perten_bits) - 1; // allow subsecond ticks to work on it
				p.excess = p.excess - std::min(perten_base, perten_apply(p.excess, e.power_status(power_excess_loss_rate, i)));
			}
			// second damage
			for (unsigned damage_type = 0; damage_type < e.damage.size(); damage_type++)
			{
				damage_def& d = e.damage[damage_type];
				int amount = perten_apply(d.dot[current_effect_second], e.damage_status(damage_dot, damage_type));
				d.dot[current_effect_second] = 0;
				// we're assuming power zero is most important, ie health, so going in reverse order
				for (int power_type = (int)e.powers.size() - 1; power_type >= 0; power_type--)
				{
					power_def& p = e.powers[power_type];
					int share = perten_apply(amount, e.damage_distribution(damage_type, power_type));
					amount -= share;
					share = perten_apply(share, e.power_status(power_dot_taken, power_type)); // apply power-related damage mod
					amount += p.reduce(share); // roll leftover damage "back in" to dealt with by other powers
				}
			}
		}
		current_effect_second = next_effect;
	}

	int current_effect_second = 0; // rotates through [0 ... max_effect_secs] continuously

	void self_test()
	{
		for (entity& e : entities)
		{
			for (unsigned i = 0; i < e.powers.size(); i++)
			{
				power_def& p = e.powers[i];
				unsigned maximum = e.power_status(power_maximum, i);
				assert(p.current <= maximum);

				uint32_t current;
				uint32_t maximum2;
				uint32_t recovering;
				uint32_t losing;
				e.power_bar(i, maximum2, current, recovering, losing);
				assert(maximum == maximum2);
				assert(maximum >= recovering);
				assert(losing <= current);
				assert(current <= maximum);
			}
			for (unsigned damage_type = 0; damage_type < e.damage.size(); damage_type++)
			{
				assert(e.damage_distribution(damage_type, 0) == perten_base); // not allowed to reduce first power distribution
			}
		}
	}
};

};
