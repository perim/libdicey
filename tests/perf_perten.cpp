#include "perten.h"
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <algorithm>
#include <limits>
#include <array>

static perten a = perten_empty;

int main(int argc, char **argv)
{
	lazy_conditional_matrix<128, 128> m;
	for (int i = 0; i < 128; i++)
	{
		for (int j = 0; j < 128; j++)
		{
			m.modify(i, j, perten{140});
		}
	}
	for (int i = 0; i < 128; i++)
	{
		for (int j = 0; j < 128; j++) perten_apply(a, m.row(j));
		for (int j = 0; j < 128; j++) perten_apply(a, m.row(j));
		for (int j = 0; j < 128; j++) perten_apply(a, m.row(j));
	}

	return perten_to_int(a);
}
