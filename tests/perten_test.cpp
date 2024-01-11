#include "perten.h"
#include <assert.h>
#include <stdio.h>

static void test_drain()
{
	perten amount;
	perten source;
	amount = perten_full; source = perten_full; perten_drain(amount, source); assert(amount == perten_empty); assert(source == perten_empty);
	amount = perten_half; source = perten_full; perten_drain(amount, source); assert(amount == perten_empty); assert(source == perten_half);
	amount = perten_full; source = perten_half; perten_drain(amount, source); assert(amount == perten_half); assert(source == perten_empty);
}

static bool nearly_equalf(double a, double b)
{
	return (fabs(a - b) < 0.2 && fabs(a - b) < 0.2);
}

static bool nearly_equali(int a, int b)
{
	return (a - 1 <= b && a + 1 >= b);
}

static void inc(perten base, perten mod, perten expected)
{
	if (perten_increase(base, mod) != expected)
	{
		printf("FAILED: perten_increase(%d%%, %d%%) -> %d%% (expected %d%%)\n", perten_to_percent(base), perten_to_percent(mod), perten_to_percent(perten_increase(base, mod)), perten_to_percent(expected));
		abort();
	}
}

static void dec(perten base, perten mod, perten expected)
{
	if (perten_reduce(base, mod) != expected)
	{
		printf("FAILED: perten_reduce(%d%%, %d%%) -> %d%% (expected %d%%)\n", perten_to_percent(base), perten_to_percent(mod), perten_to_percent(perten_reduce(base, mod)), perten_to_percent(expected));
		abort();
	}
}

static void apply(perten base, perten mod, perten expected)
{
	if (perten_apply(base, mod) != expected)
	{
		printf("FAILED: perten_apply(%d%%, %d%%) -> %d%% (expected %d%%)\n", perten_to_percent(base), perten_to_percent(mod), perten_to_percent(perten_apply(base, mod)), perten_to_percent(expected));
		abort();
	}
}

static void perten_test()
{
	test_drain();

	inc(perten_full, perten_full, perten_double);
	inc(perten_full, perten_double, perten_triple);
	inc(perten_max, perten_full, perten_max);
	inc(perten_max, perten_half, perten_max);
	inc(perten_eight, perten_full, perten_quarter);

	dec(perten_eight, perten_full, perten_empty);
	dec(perten_full, perten_full, perten_empty);
	dec(perten_eight, perten_half, perten_16th);
	dec(perten_max, perten_full, perten_empty);

	for (int i = 0; i < 2000; i++) { float f = i * 0.1f; assert(nearly_equalf(f, perten_to_percentf(perten_from_percent(f)))); }
	for (int i = 0; i < 16400; i++) { assert(i == perten_to_percent(perten_from_percent(i))); }
	for (perten i{0}; i.value < UINT16_MAX; i.value++) { perten v = perten_from_percent(perten_to_percentf(i)); assert(nearly_equali(i.value, v.value)); }
	apply(perten_full, perten_empty, perten_empty);
	apply(perten_full, perten_full, perten_full);
	apply(perten_full, perten_double, perten_double);
	apply(perten_full, perten_half, perten_half);
	apply(perten_half, perten_full, perten_half);
	apply(perten_half, perten_half, perten_quarter);
	apply(perten_quarter, perten_full, perten_quarter);
	apply(perten_from_percent(16), perten_full, perten_from_percent(16));
	apply(perten_full, perten{perten_base + 1}, perten{perten_base + 1});
	apply(perten{UINT32_MAX}, perten{perten_base + 1}, perten{UINT32_MAX});
	apply(perten{UINT32_MAX}, perten{UINT32_MAX}, perten{UINT32_MAX});

	perten pool = perten_full;
	perten amount = perten_half;
	perten_drain(pool, amount);
	assert(pool == perten_half);
	assert(amount == perten_empty);

	perten pool1 = perten_full;
	perten pool2 = perten_full;
	amount = perten_double;
	perten_drain(pool1, amount);
	assert(pool1 == perten_empty);
	assert(amount == perten_full);
	perten_drain(pool2, amount);
	assert(pool2 == perten_empty);
	assert(amount == perten_empty);
}

static void test_condmatrix()
{
	lazy_conditional_matrix<16, 16> matrix;
	for (int i = 0; i < 16; i++) assert(matrix.is_enabled(i) == false);
	for (int i = 0; i < 16; i++) assert(matrix.row(i) == perten_full); // cached zero value
	matrix.modify(0, 0, perten_32th);
	assert(matrix.row(0) == perten_full); // not yet enabled
	matrix.toggle(0, true);
	assert(matrix.row(0) == perten_32th); // now 16
	assert(matrix.row(0) == perten_32th); // cached 16
	matrix.modify(1, 0, perten_double);
	matrix.modify(2, 0, perten_double);
	matrix.modify(3, 0, perten_16th);
	matrix.modify(4, 0, perten_16th);
	matrix.toggle(0, true);
	matrix.toggle(1, true);
	assert(matrix.row(0) == perten_16th);
}

int main(int argc, char **argv)
{
	perten_test();
	test_condmatrix();

	return 0;
}
