#include "dice.h"

#include <queue>
#include <assert.h>
#include <stdio.h>
#include <chrono>

// Inspired by https://github.com/cdanek/KaimiraWeightedList
const_roll_table::const_roll_table(const std::vector<int>& weights) : size(weights.size()), alias(size), probability(size), sum(std::accumulate(weights.cbegin(), weights.cend(), 0))
{
	std::fill(alias.begin(), alias.end(), -1);
	std::queue<int> small;
	std::queue<int> large;
	std::vector<int> w;
	for (int i : weights)
	{
		w.push_back(i * size);
	}
	for (int i = 0; i < size; i++)
	{
		if (w.at(i) < sum) { small.push(i); }
		else { large.push(i); }
	}
	while (small.size() > 0 && large.size() > 0)
	{
		const int lv = large.front(); large.pop();
		const int sv = small.front(); small.pop();
		probability[sv] = w.at(sv);
		alias[sv] = lv;
		const int tmp = w[lv] + w[sv] - sum;
		w[lv] = tmp;
		if (tmp < sum) small.push(lv);
		else large.push(lv);
	}
	while (large.size() > 0)
	{
		int lv = large.front(); large.pop();
		probability[lv] = sum;
	}
}

// here so we do not have to include the chrono header in our public header
seed seed_random()
{
	uint64_t v;
	do
	{
		v = static_cast<uint64_t>(std::chrono::high_resolution_clock::now().time_since_epoch().count());
	} while (v == 0);
	return seed(v);
}

roll_table::roll_table(const seed& orig, const std::vector<int>& input) : s(orig)
{
	// Sort by weight
	std::multimap<int, int> tmp;
	for (unsigned i = 0; i < input.size(); i++)
	{
		tmp.emplace(input.at(i), i);
	}
	// Create roll table
	size = -1; // to account for a zero roll result
	for (auto iter = tmp.rbegin(); iter != tmp.rend(); ++iter)
	{
		size += (*iter).first;
		table[size] = (*iter).second;
	}
	size--;
}

luck_type luck_combine(luck_type l, luck_type against)
{
	switch (against)
	{
	case luck_type::mediocre:
		if (l == luck_type::uncommon) l = luck_type::normal;
		else if (l == luck_type::very_lucky) l = luck_type::lucky;
		else if (l == luck_type::lucky) l = luck_type::normal;
		else l = against;
		break;
	case luck_type::uncommon:
		if (l == luck_type::mediocre) l = luck_type::normal;
		else if (l == luck_type::very_unlucky) l = luck_type::unlucky;
		else if (l == luck_type::unlucky) l = luck_type::normal;
		else l = against;
		break;
	case luck_type::lucky:
		if (l == luck_type::normal) l = luck_type::unlucky;
		else if (l == luck_type::lucky) l = luck_type::normal;
		else if (l == luck_type::very_lucky) l = luck_type::lucky;
		else if (l == luck_type::unlucky) l = luck_type::very_unlucky;
		break;
	case luck_type::unlucky:
		if (l == luck_type::normal) l = luck_type::lucky;
		else if (l == luck_type::lucky) l = luck_type::very_lucky;
		else if (l == luck_type::very_unlucky) l = luck_type::unlucky;
		else if (l == luck_type::unlucky) l = luck_type::normal;
		break;
	case luck_type::very_lucky:
		if (l == luck_type::normal || l == luck_type::unlucky) l = luck_type::very_unlucky;
		else if (l == luck_type::very_lucky) l = luck_type::normal;
		else if (l == luck_type::lucky) l = luck_type::unlucky;
		break;
	case luck_type::very_unlucky:
		if (l == luck_type::normal || l == luck_type::lucky) l = luck_type::very_lucky;
		else if (l == luck_type::very_unlucky) l = luck_type::normal;
		else if (l == luck_type::unlucky) l = luck_type::lucky;
		break;
	default: break;
	}
	return l;
}

int seed::roll(int low, int high, luck_type luck, int jackpot_chance, int jackpot_low, int jackpot_high, luck_type jackpot_luck)
{
	const int avg = (high - low) / 2;
	int v1 = roll(low, high);
	if (luck == luck_type::normal)
	{
		// nothing extra
	}
	else if (luck == luck_type::mediocre)
	{
		const int v2 = roll(low, high);
		const int dist1 = v1 - low - avg;
		const int dist2 = v2 - low - avg;
		if (dist1*dist1 > dist2*dist2) v1 = v2;
	}
 	else if (luck == luck_type::uncommon)
	{
		const int v2 = roll(low, high);
		const int dist1 = v1 - low - avg;
		const int dist2 = v2 - low - avg;
		if (dist1*dist1 < dist2*dist2) v1 = v2;
	}
 	else if (luck == luck_type::lucky)
	{
		const int v2 = roll(low, high);
		if (v2 > v1) v1 = v2;
	}
 	else if (luck == luck_type::unlucky)
	{
		const int v2 = roll(low, high);
		if (v2 < v1) v1 = v2;
	}
 	else if (luck == luck_type::very_lucky)
	{
		const int v2 = roll(low, high);
		const int v3 = roll(low, high);
		if (v2 > v1) v1 = v2;
		if (v3 > v1) v1 = v3;
	}
 	else if (luck == luck_type::very_unlucky)
	{
		const int v2 = roll(low, high);
		const int v3 = roll(low, high);
		if (v2 < v1) v1 = v2;
		if (v3 < v1) v1 = v3;
	}

	if (v1 > high - jackpot_chance)
	{
		v1 += roll(jackpot_low, jackpot_high, jackpot_luck);
	}

	return v1;
}

int roll_table::unique_rolls(int count, int* results, luck_type rollee_luck, int roll_weight, int start_index)
{
	roll_weight = (size * roll_weight) >> 7;
	for (int i = 0; i < count; i++)
	{
		if ((int)table.size() <= i + start_index) return i; // ran out of options
repeat:
		const int r = s.roll(roll_weight, size - roll_weight, rollee_luck);
		const int k = lookup(r);
		for (int j = 0; j < start_index + i; j++)
		{
			if (results[j] == k) goto repeat;
		}
		results[start_index + i] = k;
	}
	return count;
}

int roll_table::rolls(int count, int* results, luck_type rollee_luck, int roll_weight)
{
	roll_weight = (size * roll_weight) >> 7;
	for (int i = 0; i < count; i++)
	{
		const int r = s.roll(roll_weight, size - roll_weight, rollee_luck);
		const int k = lookup(r);
		results[i] = k;
	}
	return count;
}

int roll_table::boxgacha(luck_type rollee_luck, int roll_weight)
{
	if (table.size() == 0) return -1;
	roll_weight = (size * roll_weight) >> 7;
	const int r = s.roll(roll_weight, size - roll_weight, rollee_luck);
	auto it = table.upper_bound(r);
	if (it == table.end()) it = table.begin();
	const int ret = (*it).second;
	table.erase((*it).first);
	return ret;
}
