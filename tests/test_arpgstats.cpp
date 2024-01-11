#include "arpgstats.h"

#include <stdio.h>

static void test_over_time_effects()
{
	assert(perten_from_percent(100) == perten_full);
	arpg::damage_mods offense;
	offense.fill(100);
	arpg::stats s;
	s.entities.push_back({});
	s.entities[0].apply_damage_over_time(s.current_effect_second, 0, 3, 16, offense);
	assert(s.entities[0].powers[0].current == perten_full);
	assert(s.entities[0].damage[0].dot[0] == perten_from_percent(16));
	assert(s.entities[0].damage[0].dot[1] == perten_from_percent(16));
	assert(s.entities[0].damage[0].dot[2] == perten_from_percent(16));
	assert(s.entities[0].damage[0].dot[3] == perten_empty);
	assert(s.entities[0].damage[1].dot[0] == perten_empty);
	assert(s.entities[0].powers[0].recovery[s.current_effect_second] == perten_empty);
	s.second_tick();
	assert(s.entities[0].powers[0].recovery[s.current_effect_second] == perten_empty);
	assert(s.entities[0].damage[0].dot[0] == perten_empty); // this should have been emptied
	assert(s.entities[0].damage[0].dot[1] == perten_from_percent(16)); // second dot slot should still be filled
	assert(s.current_effect_second == 1); // we should be in tick number two
	assert(perten_to_percent(s.entities[0].powers[0].current) == 100 - 16); // we should have lost 16% of health
	s.second_tick();
	assert(s.current_effect_second == 2);
	assert(perten_to_percent(s.entities[0].powers[0].current) == 100 - 32);
	s.second_tick();
	assert(s.current_effect_second == 3);
	assert(perten_to_percent(s.entities[0].powers[0].current) == 100 - 48);
	s.second_tick();
	assert(s.current_effect_second == 4);
	assert(perten_to_percent(s.entities[0].powers[0].current) == 100 - 48); // expired, no further damage

	s.entities[0].apply_damage_over_time(s.current_effect_second, 0, 1, 5000, offense);
	assert(s.entities[0].damage[0].dot[4] == perten_from_percent(5000));
	assert(s.entities[0].damage[0].dot[5] == perten_empty);
	s.second_tick();
	assert(s.current_effect_second == 5);
	assert(s.entities[0].powers[0].current == perten_empty); // dead

	s.entities[0].powers[0].current = perten_half; // instantly heal up to half standard health

	// apply healing over time
	s.entities[0].apply_recover_effect(s.current_effect_second, 0, 2, 25);
	assert(s.entities[0].powers[0].current == perten_from_percent(50));
	s.second_tick();
	assert(s.entities[0].powers[0].current == perten_from_percent(75));
	s.second_tick();
	assert(s.entities[0].powers[0].current == perten_from_percent(100));
	s.second_tick();
	assert(s.entities[0].powers[0].current == perten_from_percent(100));

	s.self_test();
}

static void test_damage_distribution()
{
	arpg::damage_mods offense;
	offense.fill(100);
	arpg::stats s;
	s.entities.push_back({});
	s.entities[0].statuses[arpg::damage_distribution_index(0, 1)] = perten_half; // take half from power 2
	s.entities[0].statuses[arpg::damage_status_index(arpg::damage_dot, 0)] = perten_half; // halve
	s.entities[0].apply_damage_over_time(s.current_effect_second, 0, 2, 100, offense);
	assert(s.entities[0].damage[0].dot[0] == perten_full);
	assert(s.entities[0].damage[0].dot[1] == perten_full);
	s.second_tick();
	assert(perten_to_percent(s.entities[0].powers[0].current) == 75);
	assert(perten_to_percent(s.entities[0].powers[1].current) == 75);
	assert(s.entities[0].powers[2].current == perten_full);
	s.second_tick();
	assert(perten_to_percent(s.entities[0].powers[0].current) == 50);
	assert(perten_to_percent(s.entities[0].powers[1].current) == 50);
	assert(s.entities[0].powers[2].current == perten_full);

	s.self_test();
}

static void test_damage()
{
	// two entities, one damaging the other; damaging entity has an item giving double damage,
	// damaged entity has a temporary curse that doubles the dot time taken
	arpg::damage_mods_full offense;
	for (auto& v : offense) v.fill(100);
	arpg::stats s;
	s.entities.push_back({}); // attacker
	s.entities.push_back({}); // defender
	arpg::damage_also_dealt_as instant = { 0, 0, 32 };
	arpg::damage_also_dealt_as dot = { 64, 0 };
	// offense. // item
	s.entities[1].timed_status_increase(arpg::damage_status_index(arpg::damage_dot, 1), 256, 2); // curse
	int survived = s.entities[1].apply_damage_full(s.current_effect_second, 1, 50, offense, instant, dot);
	assert(survived == 0);

	s.self_test();
}

static void test_skill()
{
	arpg::stats s;
	s.entities.push_back({});
	arpg::skill sk;
	sk.skill = 0;
	sk.cost_type = 0;
	sk.cost = 10;
	sk.flags = 0;
	sk.windup_time = 10;
	sk.animation_time = 0;
	sk.cooldown_time = 20;
	s.entities[0].set_skill(0, sk);
	assert(s.entities[0].skill_ready(0) == true);

	s.self_test();
}

int main()
{
	test_over_time_effects();
	test_damage_distribution();
	test_damage();
	test_skill();
	return 0;
}
