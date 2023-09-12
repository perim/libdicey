#include "chunky.h"
#include <assert.h>
#include <stdio.h>
#include <time.h>

static void test_inner_loop(seed s, bool debug = false)
{
	chunkconfig config(s);
	config.width = 64;
	config.height = 32;
	config.level_width = 4;
	config.level_height = 4;
	config.x = s.roll(0, config.level_width - 1);
	config.y = s.roll(0, config.level_height - 1);

	chunk c(config);
	c.generate_exits();
	bool r = chunk_filter_connect_exits_inner_loop(c);
	if (r && debug)
	{
		printf("Inner loop seed=%lu:\n", (unsigned long)s.state);
		c.print_chunk();
	}
}

static void test_grand_central(seed s, bool debug = false)
{
	chunkconfig config(s);
	config.width = 64;
	config.height = 32;
	config.level_width = 4;
	config.level_height = 4;
	config.x = s.roll(0, config.level_width - 1);
	config.y = s.roll(0, config.level_height - 1);

	chunk c(config);
	c.generate_exits();
	bool r = chunk_filter_connect_exits_grand_central(c);
	if (r && debug)
	{
		c.print_chunk();
	}
}

static void exit_test_case(int width, int height, int left, int right, int top, int bottom)
{
	seed s(0);
	chunkconfig config(s);
	config.width = width;
	config.height = height;
	config.level_width = 4;
	config.level_height = 4;
	config.x = 1;
	config.y = 1;

	chunk c(config);
	if (left != -1) c.make_exit_left(left);
	if (right != -1) c.make_exit_right(right);
	if (top != -1) c.make_exit_top(top);
	if (bottom != -1) c.make_exit_bottom(bottom);
	chunk_filter_connect_exits(c);
	chunk_filter_room_expand(c, 3, 6);
	assert(c.rooms.size() > 4);
}

static void exit_test()
{
	exit_test_case(32, 32, 8, 8, 8, -1);
	exit_test_case(32, 32, 8, 8, -1, -1);
	exit_test_case(32, 32, 3, -1, 3, -1);
	exit_test_case(32, 32, 30, 30, 3, -1);
	exit_test_case(32, 32, 3, 30, 3, 30);
	exit_test_case(32, 32, 30, 30, 30, 30);
	exit_test_case(64, 32, -1, 29, -1, 38);
}

static void connect_test()
{
	seed s(0);
	chunkconfig config(s);
	config.level_width = 2;
	config.level_height = 2;
	config.x = 0;
	config.y = 0;
	chunk c1(config);
	c1.generate_exits();
	config.x = 1;
	config.y = 0;
	chunk c2(config);
	c2.generate_exits();
	config.x = 0;
	config.y = 1;
	chunk c3(config);
	c3.generate_exits();
	config.x = 1;
	config.y = 1;
	chunk c4(config);
	c4.generate_exits();

	c1.self_test();
	c2.self_test();
	c3.self_test();
	c4.self_test();

	assert(c1.top == -1);
	assert(c1.left == -1);
	assert(c2.top == -1);
	assert(c2.right == -1);
	assert(c3.left == -1);
	assert(c3.bottom == -1);
	assert(c4.right == -1);
	assert(c4.bottom == -1);

	assert(c1.bottom == c3.top);
	assert(c1.right == c2.left);
	assert(c2.bottom == c4.top);
	assert(c3.right == c4.left);
}

static void stress_test(seed s, bool debug)
{
	chunkconfig config(s);
	config.chaos = s.roll(0, 4);
	config.openness = s.roll(0, 4);
	config.width = 1 << s.roll(6, 7);
	config.height = 1 << s.roll(5, 6);
	config.level_width = 4;
	config.level_height = 4;
	config.x = s.roll(0, config.level_width - 1);
	config.y = s.roll(0, config.level_height - 1);
	chunk c(config);
	c.generate_exits();
	assert(config.state.state != s.state);
	c.self_test();
	chunk_filter_connect_exits(c);
	const int iter = s.roll(2, 8);
	chunk_filter_room_expand(c, iter, iter + 6);
	c.self_test();
	chunk_filter_room_in_room(c);
	c.self_test();
	chunk_filter_one_way_doors(c, s.roll(0, 4));
	c.beautify();
	room r = chunk_filter_boss_placement(c, 0);
	chunk_filter_protect_room(c, r);
	chunk_filter_wildlife(c);
	if (debug) print_room(c, c.room_index(r));
}

int main(int argc, char **argv)
{
	test_grand_central(seed_random());
	test_inner_loop(seed_random());
	exit_test();
	connect_test();

	// Visual test
	uint64_t value = time(nullptr);
	if (argc > 1) value = atoll(argv[1]);
	printf("Showing room from seed %llu:\n", (unsigned long long)value);
	stress_test(seed(value, value), true);

	// Stress test - deterministic
	for (int i = 0; i < 256; i++)
	{
		stress_test(seed(i), false);
	}

	// Stress test - random
	for (int i = 0; i < 256; i++)
	{
		stress_test(seed_random(), false);
	}

	return 0;
}
