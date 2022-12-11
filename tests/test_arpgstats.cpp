#include "arpgstats.h"

#include <stdio.h>

static void test_simple()
{
	int amount;
	uint16_t source;
	amount = 40; source = 100; arpg::take(amount, source); assert(amount == 0); assert(source == 60);
	amount = 100; source = 40; arpg::take(amount, source); assert(amount == 60); assert(source == 0);
	amount = 50; source = 50; arpg::take(amount, source); assert(amount == 0); assert(source == 0);

	arpg::damage_mods offense;
	offense.fill(perten_base);
	arpg::stats s;
	s.entities.push_back({});
	s.entities[0].apply_damage_over_time(s.current_effect_second, 0, 3, 16, offense);
	s.entities[0].power_status(arpg::power_regeneration_rate, 0);
	assert(s.entities[0].damage[0].dot[0] == 16);
	assert(s.entities[0].damage[0].dot[1] == 16);
	assert(s.entities[0].damage[0].dot[2] == 16);
	assert(s.entities[0].damage[0].dot[3] == 0);
	assert(s.entities[0].damage[1].dot[0] == 0);
	s.second_tick();
	assert(s.entities[0].powers[0].recovery[s.current_effect_second] == 0);
	assert(s.current_effect_second == 1);
	assert(s.entities[0].powers[0].current == perten_base - 16);
	s.second_tick();
	assert(s.current_effect_second == 2);
	assert(s.entities[0].powers[0].current == perten_base - 32);
	s.second_tick();
	assert(s.current_effect_second == 3);
	assert(s.entities[0].powers[0].current == perten_base - 48);
	s.second_tick();
	assert(s.current_effect_second == 4);
	assert(s.entities[0].powers[0].current == perten_base - 48); // expired, no further damage

	s.entities[0].apply_damage_over_time(s.current_effect_second, 0, 1, 5000, offense);
	assert(s.entities[0].damage[0].dot[4] == 5000);
	assert(s.entities[0].damage[0].dot[5] == 0);
	s.second_tick();
	assert(s.current_effect_second == 5);
	assert(s.entities[0].powers[0].current == 0); // dead

	s.entities[0].powers[0].current = 50; // instantly heal up to 50

	// apply healing over time
	s.entities[0].apply_recover_effect(s.current_effect_second, 0, 4, 250);
	s.second_tick();
	assert(s.entities[0].powers[0].current == 300);
	s.second_tick();
	assert(s.entities[0].powers[0].current == 550);
	s.second_tick();
	assert(s.entities[0].powers[0].current == 800);
	s.second_tick();
	assert(s.entities[0].powers[0].current == perten_base);

	s.self_test();
}

static void test_2()
{
	arpg::damage_mods offense;
	offense.fill(perten_base);
	arpg::stats s;
	s.entities.push_back({});
	s.entities[0].statuses[arpg::damage_distribution_index(0, 1)] = perten_base / 2; // take half from power 2
	s.entities[0].statuses[arpg::damage_status_index(arpg::damage_dot, 0)] = perten_base / 2; // halve
	s.entities[0].apply_damage_over_time(s.current_effect_second, 0, 2, 100, offense);
	assert(s.entities[0].damage[0].dot[0] == 100);
	assert(s.entities[0].damage[0].dot[1] == 100);
	s.second_tick();
	assert(s.entities[0].powers[0].current == perten_base - 25);
	assert(s.entities[0].powers[1].current == perten_base - 25);
	assert(s.entities[0].powers[2].current == perten_base);
	s.second_tick();
	assert(s.entities[0].powers[0].current == perten_base - 50);
	assert(s.entities[0].powers[1].current == perten_base - 50);
	assert(s.entities[0].powers[2].current == perten_base);

	s.self_test();
}

static void test_3()
{
	// two entities, one damaging the other; damaging entity has an item giving double damage,
	// damaged entity has a temporary curse that doubles the dot time taken
	arpg::damage_mods_full offense;
	for (auto& v : offense) v.fill(perten_base);
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

int main()
{
	test_simple();
	test_2();
	test_3();
	return 0;
}
