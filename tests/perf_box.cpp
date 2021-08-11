#include "dice.h"
#include <assert.h>
#include <stdio.h>

// test performance of the roll table rolls() call
int main(int argc, char **argv)
{
	seed s = seed_random();
	uint64_t sum = 0;
	const int num = 50000;
	std::vector<int> t(num);
	for (int i = 1; i < num; i++) t[i] = i / num;
	roll_table* rt = roll_table_make(t);
	for (int i = 1; i < num; i++)
	{
		sum += s.boxgacha(rt, luck_type::normal, 0);
	}
	roll_table_free(rt);
	return (int)sum * 0;
}
