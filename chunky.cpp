#include "chunky.h"

#include <stdio.h>

static bool debug = false;

chunk::chunk(const chunkconfig& c) : width(c.width), height(c.height), config(c), map(c.width * c.height)
{
	assert(ispow2(width));
	bits = highestbitset(width);
}

void chunk::room_list_self_test() const
{
	for (unsigned i = 0; i < rooms.size(); i++)
	{
		const room& r1 = rooms.at(i);
		r1.self_test();
		assert(r1.x2 >= r1.x1 && r1.x1 > 0 && r1.x2 < width - 1);
		assert(r1.y2 >= r1.y1 && r1.y1 > 0 && r1.y2 < height - 1);
		for (unsigned j = i + 1; j < rooms.size(); j++)
		{
			const room& r2 = rooms.at(j);
			assert(r1 != r2);
		}
	}
}

void chunk::self_test() const
{
	assert(ispow2(height) && ispow2(height));
	assert(rock(0, 0) && rock(width - 1, 0));
	assert(rock(0, height - 1) && rock(width - 1, height - 1));
}

void room::self_test() const
{
	assert(top != 0 && bottom != 0 && left != 0 && right != 0);
	assert(top == -1 || (top >= x1 && top <= x2));
	assert(bottom == -1 || (bottom >= x1 && bottom <= x2));
	assert(left == -1 || (left >= y1 && left <= y2));
	assert(right == -1 || (right >= y1 && right <= y2));
	assert(isolation >= 0);
	assert(valid());
}

static bool can_build(const chunk& c, int x1, int y1, int x2, int y2)
{
	assert(x2 >= x1 && y2 >= y1); // room must be valid
	if (x1 < 1 || y1 < 1 || x2 >= c.width - 1 || y2 >= c.height - 1) return false;
	for (int xx = x1 - 1; xx <= x2 + 1; xx++)
		for (int yy = y1 - 1; yy <= y2 + 1; yy++)
			if (!c.rock(xx, yy)) return false;
	return true;
}

static bool can_build(const chunk& c, const room& r)
{
	return can_build(c, r.x1, r.y1, r.x2, r.y2);
}

static bool try_grow_left(chunk& c, room& r)
{
	if (r.y1 == 0 || r.y2 == c.height - 1 || r.x1 <= 1) return false;
	for (int i = r.y1 - 1; i <= r.y2 + 1; i++) if (i != r.left && (!c.rock(r.x1 - 1, i) || !c.rock(r.x1 - 2, i))) return false;
	for (int i = r.y1; i <= r.y2; i++) c.dig(r.x1 - 1, i);
	r.x1--;
	return true;
}

static bool try_grow_right(chunk& c, room& r)
{
	if (r.y1 == 0 || r.y2 >= c.height - 1 || r.x2 >= c.width - 2) return false;
	for (int i = r.y1 - 1; i <= r.y2 + 1; i++) if (i != r.right && (!c.rock(r.x2 + 1, i) || !c.rock(r.x2 + 2, i))) return false;
	for (int i = r.y1; i <= r.y2; i++) c.dig(r.x2 + 1, i);
	r.x2++;
	return true;
}

static bool try_grow_top(chunk& c, room& r)
{
	if (r.x1 == 0 || r.x2 >= c.width - 1 || r.y1 <= 1) return false;
	for (int i = r.x1 - 1; i <= r.x2 + 1; i++) if (i != r.top && (!c.rock(i, r.y1 - 1) || !c.rock(i, r.y1 - 2))) return false;
	for (int i = r.x1; i <= r.x2; i++) c.dig(i, r.y1 - 1);
	r.y1--;
	return true;
}

static bool try_grow_bottom(chunk& c, room& r)
{
	if (r.x1 == 0 || r.x2 >= c.width - 1 || r.y2 >= c.height - 2) return false;
	for (int i = r.x1 - 1; i <= r.x2 + 1; i++) if (i != r.bottom && (!c.rock(i, r.y2 + 1) || !c.rock(i, r.y2 + 2))) return false;
	for (int i = r.x1; i <= r.x2; i++) c.dig(i, r.y2 + 1);
	r.y2++;
	return true;
}

bool chunk_room_grow(chunk& c, room& r)
{
	bool res = try_grow_left(c, r);
	res = try_grow_right(c, r) || res;
	res = try_grow_top(c, r) || res;
	return try_grow_bottom(c, r) || res;
}

void chunk_room_grow_randomly(chunk& c, room& r, int min, int max)
{
	const int iter = c.roll(min, max);
	for (int i = 0; i < iter; i++)
	{
		bool res = false;
		const int dir = c.roll(0, 4);
		switch (dir)
		{
		case 0:
			res = try_grow_left(c, r);
			res = try_grow_top(c, r) || res;
			if (!res) res = try_grow_right(c, r);
			if (!res) res = try_grow_bottom(c, r);
			break;
		case 1:
			res = try_grow_right(c, r);
			res = try_grow_top(c, r) || res;
			if (!res) res = try_grow_bottom(c, r);
			if (!res) res = try_grow_left(c, r);
			break;
		case 2:
			res = try_grow_top(c, r);
			res = try_grow_left(c, r) || res;
			if (!res) res = try_grow_bottom(c, r);
			if (!res) res = try_grow_right(c, r);
			break;
		case 3:
			res = try_grow_bottom(c, r);
			res = try_grow_right(c, r) || res;
			if (!res) res = try_grow_left(c, r);
			if (!res) res = try_grow_top(c, r);
			break;
		case 4:
			res = try_grow_top(c, r);
			res = try_grow_left(c, r) || res;
			res = try_grow_right(c, r) || res;
			res = try_grow_bottom(c, r) || res;
			break;
		}
		if (!res) break;
	}
	const int iter2 = c.roll(0, c.config.chaos);
	for (int i = 0; i < iter2; i++)
	{
		const int dir = c.roll(0, 3);
		switch (dir)
		{
		case 0: if (r.size() > min * min) chunk_room_shrink_left(c, r); break;
		case 1: if (r.size() > min * min) chunk_room_shrink_right(c, r); break;
		case 2: if (r.size() > min * min) chunk_room_shrink_top(c, r); break;
		case 3: if (r.size() > min * min) chunk_room_shrink_bottom(c, r); break;
		}
	}
}

room chunk_room_make(chunk& c, int times, int x, int y)
{
	room r(x, y, x, y, 0, c.roll(0, c.config.chaos) == 0 ? ROOM_FLAG_NEAT : 0);
	if (x > 0 && !c.rock(x - 1, y)) r.left = y;
	if (x < c.width - 1 && !c.rock(x + 1, y)) r.right = y;
	if (y > 0 && !c.rock(x, y - 1)) r.top = x;
	if (y < c.height - 1 && !c.rock(x, y + 1)) r.bottom = x;
	c.dig(x, y);
	for (int i = 0; i < times; i++) if (!chunk_room_grow(c, r)) return r;
	r.self_test();
	return r;
}

void chunk_filter_connect_exits(chunk& c)
{
	c.self_test();
	assert(c.rooms.size() == 0);
	int j;

	// top
	if (c.top != -1)
	{
		int limit = std::max(c.left, c.right);
		if (limit == -1) limit = c.height >> 1;
		for (j = 1; j <= limit; j ++) c.dig(c.top, j);
		if (j > 2) { c.rooms.emplace_back(c.top, 1, c.top, j - 1, 0, ROOM_FLAG_CORRIDOR); assert(c.rooms.back().valid()); }
	}

	// bottom
	if (c.bottom != -1)
	{
		const short middle = c.height >> 1;
		int limit = c.left;
		if (limit == -1 || (c.right != -1 && c.right < limit)) limit = c.right;
		if (limit == -1) limit = middle;
		for (j = c.height - 2; j >= limit; j--) c.dig(c.bottom, j);
		if (j < c.height - 4) { c.rooms.emplace_back(c.bottom, std::min(j + 1, c.height - 2), c.bottom, c.height - 2, 0, ROOM_FLAG_CORRIDOR); assert(c.rooms.back().valid()); }
		// add a bridge?
		if (c.top != -1 && c.left == -1 && c.right == -1 && c.top != c.bottom)
		{
			const short min = std::min(c.top, c.bottom);
			for (j = min; j < std::max(c.top, c.bottom) && c.rock(j, middle); j++) c.dig(j, middle);
			if (j - min > 2) { c.rooms.emplace_back(min, middle, j - 1, middle, 0, ROOM_FLAG_CORRIDOR); assert(c.rooms.back().valid()); }
		}
	}

	// left
	if (c.left != -1)
	{
		int limit = std::max(c.top, c.bottom);
		if (limit == -1) limit = c.width >> 1;
		for (j = 1; j <= limit; j++) c.dig(j, c.left);
		if (j > 2) { c.rooms.emplace_back(1, c.left, j - 1, c.left, 0, ROOM_FLAG_CORRIDOR); assert(c.rooms.back().valid()); }
	}

	// right
	if (c.right != -1)
	{
		const short middle = c.width >> 1;
		int limit = c.top;
		if (limit == -1 || (c.bottom != -1 && c.bottom < limit)) limit = c.bottom;
		if (limit == -1) limit = middle;
		for (j = c.width - 2; j >= limit; j--) c.dig(j, c.right);
		if (j < c.width - 4) { c.rooms.emplace_back(std::min(c.width - 2, j + 1), c.right, c.width - 2, c.right, 0, ROOM_FLAG_CORRIDOR); assert(c.rooms.back().valid()); }
		// add a bridge?
		if (c.left != -1 && c.top == -1 && c.bottom == -1 && c.left != c.right)
		{
			const short min = std::min(c.left, c.right);
			for (j = std::min(c.left, c.right); j < std::max(c.left, c.right); j++) c.dig(middle, j);
			if (j - min > 2) { c.rooms.emplace_back(middle, min, middle, j - 1, 0, ROOM_FLAG_CORRIDOR); assert(c.rooms.back().valid()); }
		}
	}

	assert(c.rooms.size() > 0);
	c.room_list_self_test();
}

static inline void print_tile(uint8_t t)
{
	switch (t)
	{
	case TILE_ROCK: printf(" "); break;
	case TILE_WALL: printf("#"); break;
	case TILE_DOOR: printf("+"); break;
	case TILE_ONE_WAY_TOP: printf("^"); break;
	case TILE_ONE_WAY_BOTTOM: printf("_"); break;
	case TILE_ONE_WAY_RIGHT: printf("]"); break;
	case TILE_ONE_WAY_LEFT: printf("["); break;
	case TILE_EMPTY: printf("."); break;
	default: assert(false); break;
	}
}

void chunk::print_chunk() const
{
	printf("Chunk (width=%d, height=%d) (top=%d, right=%d, bottom=%d, left=%d)\n", width, height, top, right, bottom, left);
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			print_tile(at(x, y));
		}
		printf("\n");
	}
	printf("\n");
}

void print_room(const chunk& c, const room& r)
{
	for (int y = 0; y < c.height; y++)
	{
		for (int x = 0; x < c.width; x++)
		{
			int t = c.at(x, y);
			if (x >= r.x1 && x <= r.x2 && y >= r.y1 && y <= r.y2) printf("\e[0;33m");
			else if (t >= TILE_DOOR && t <= TILE_ONE_WAY_RIGHT) printf("\e[0;36m");
			print_tile(t);
			printf("\x1b[0m");
		}
		if (y == 0) printf("\tSize: %d", r.size());
		else if (y == 1) printf("\tIsolation: %d", r.isolation);
		else if (y == 2) printf("\tArea: (%d, %d), (%d, %d)", r.x1, r.y1, r.x2, r.y2);
		else if (y == 3) printf("\tExits: top=%d, right=%d, bottom=%d, left=%d", r.top, r.right, r.bottom, r.left);
		else if (y == 4) printf("\tFlags: %d", r.flags);
		else if (y == 5) printf("\tCluster: %d", r.cluster);
		printf("\n");
	}
        printf("\n");
}
void print_room(const chunk& c, int roomidx)
{
	const room& r = c.rooms.at(roomidx);
	print_room(c, r);
}

void chunk_room_dig(chunk& c, const room& r)
{
	for (int x = r.x1; x <= r.x2; x++)
	{
		for (int y = r.y1; y <= r.y2; y++)
		{
			c.dig(x, y);
		}
	}
}

void chunk_room_draw(chunk& c, const room& r, bool door)
{
	tile_type t = door ? TILE_DOOR : TILE_EMPTY;
	for (int x = r.x1 - 1; x <= r.x2 + 1; x++) c.makewall(x, r.y1 - 1);
	for (int x = r.x1 - 1; x <= r.x2 + 1; x++) c.makewall(x, r.y2 + 1);
	for (int y = r.y1 - 1; y <= r.y2 + 1; y++) c.makewall(r.x1 - 1, y);
	for (int y = r.y1 - 1; y <= r.y2 + 1; y++) c.makewall(r.x2 + 1, y);
	if (r.top > 0) c.build(r.top, r.y1 - 1, t);
	if (r.bottom > 0) c.build(r.bottom, r.y2 + 1, t);
	if (r.left > 0) c.build(r.x1 - 1, r.left, t);
	if (r.right > 0) c.build(r.x2 + 1, r.right, t);
}

bool chunk_room_shrink_top(chunk& c, room& r)
{
	if (r.top == -1 || r.left <= r.y1 + 2 || r.right <= r.y1 + 2 || r.y2 - r.y1 < 3 || r.x2 - r.x1 < 3) return false;
	int newexit = c.roll(r.x1, r.x2);
	if (r.left != r.y1) for (int i = r.x1; i < std::min<int>(newexit, r.top); i++) c.makewall(i, r.y1);
	if (r.right != r.y1) for (int i = std::max<int>(r.top, newexit) + 1; i <= r.x2; i++) c.makewall(i, r.y1);
	for (int i = r.x1; i < newexit; i++) c.makewall(i, r.y1 + 1);
	for (int i = newexit + 1; i <= r.x2; i++) c.makewall(i, r.y1 + 1);
	r.y1 += 2;
	r.top = newexit;
	r.self_test();
	return true;
}

bool chunk_room_shrink_bottom(chunk& c, room& r)
{
	if (r.bottom == -1 || r.left >= r.y2 - 2 || r.right >= r.y2 - 2 || r.y2 - r.y1 < 3 || r.x2 - r.x1 < 3) return false;
	int newexit = c.roll(r.x1, r.x2);
	if (r.left != r.y2) for (int i = r.x1; i < std::min<int>(newexit, r.bottom); i++) c.makewall(i, r.y2);
	if (r.right != r.y2) for (int i = std::max<int>(r.bottom, newexit) + 1; i <= r.x2; i++) c.makewall(i, r.y2);
	for (int i = r.x1; i < newexit; i++) c.makewall(i, r.y2 - 1);
	for (int i = newexit + 1; i <= r.x2; i++) c.makewall(i, r.y2 - 1);
	r.y2 -= 2;
	r.bottom = newexit;
	r.self_test();
	return true;
}

bool chunk_room_shrink_right(chunk& c, room& r)
{
	if (r.right == -1 || r.bottom >= r.x2 - 2 || r.top >= r.x2 - 2 || r.x2 - r.x1 < 3 || r.y2 - r.y1 < 3) return false;
	int newexit = c.roll(r.y1, r.y2);
	if (r.top != r.x2) for (int i = r.y1; i < std::min<int>(newexit, r.right); i++) c.makewall(r.x2, i);
	if (r.bottom != r.x2) for (int i = std::max<int>(r.right, newexit) + 1; i <= r.y2; i++) c.makewall(r.x2, i);
	for (int i = r.y1; i < newexit; i++) c.makewall(r.x2 - 1, i);
	for (int i = newexit + 1; i <= r.y2; i++) c.makewall(r.x2 - 1, i);
	r.x2 -= 2;
	r.right = newexit;
	r.self_test();
	return true;
}

bool chunk_room_shrink_left(chunk& c, room& r)
{
	if (r.left == -1 || r.top <= r.x1 + 2 || r.bottom <= r.x1 + 2 || r.x2 - r.x1 < 3 || r.y2 - r.y1 < 3) return false;
	int newexit = c.roll(r.y1, r.y2);
	if (r.top != r.x1) for (int i = r.y1; i < std::min<int>(newexit, r.left); i++) c.makewall(r.x1, i);
	if (r.bottom != r.x1) for (int i = std::max<int>(r.left, newexit) + 1; i <= r.y2; i++) c.makewall(r.x1, i);
	for (int i = r.y1; i < newexit; i++) c.makewall(r.x1 + 1, i);
	for (int i = newexit + 1; i <= r.y2; i++) c.makewall(r.x1 + 1, i);
	r.x1 += 2;
	r.left = newexit;
	r.self_test();
	return true;
}

// if r1 and r2 are neighbours, and if so, where r2 is relative to r1
static int room_neighbours(const room& r1, const room& r2)
{
	if (r1.x2 + 2 == r2.x1 && range_overlap(r1.y1, r1.y2, r2.y1, r2.y2)) return DIR_RIGHT;
	else if (r1.x1 - 2 == r2.x2 && range_overlap(r1.y1, r1.y2, r2.y1, r2.y2)) return DIR_LEFT;
	else if (r1.y2 + 2 == r2.y1 && range_overlap(r1.x1, r1.x2, r2.x1, r2.x2)) return DIR_DOWN;
	else if (r1.y1 - 2 == r2.y2 && range_overlap(r1.x1, r1.x2, r2.x1, r2.x2)) return DIR_UP;
	return -1;
}

void chunk_filter_one_way_doors(chunk& c, int threshold)
{
	for (room& r : c.rooms)
	{
		for (room& r2 : c.rooms)
		{
			int side = room_neighbours(r, r2);
			if (side >= 0 && r.isolation > threshold && r.isolation - threshold > r2.isolation)
			{
				switch (side)
				{
				case DIR_RIGHT: if (r.right == -1) { r.right = r2.left = c.roll(std::max(r.y1, r2.y1), std::min(r.y2, r2.y2)); c.build(r.x2 + 1, r.right, TILE_ONE_WAY_RIGHT); } break;
				case DIR_LEFT: if (r.left == -1) { r.left = r2.right = c.roll(std::max(r.y1, r2.y1), std::min(r.y2, r2.y2)); c.build(r.x1 - 1, r.left, TILE_ONE_WAY_LEFT); } break;
				case DIR_UP: if (r.top == -1) { r.top = r2.bottom = c.roll(std::max(r.x1, r2.x1), std::min(r.x2, r2.x2)); c.build(r.top, r.y1 - 1, TILE_ONE_WAY_TOP); } break;
				case DIR_DOWN: if (r.bottom == -1) { r.bottom = r2.top = c.roll(std::max(r.x1, r2.x1), std::min(r.x2, r2.x2)); c.build(r.bottom, r.y2 + 1, TILE_ONE_WAY_BOTTOM); } break;
				default: assert(false); break;
				}
				r.self_test();
				r2.self_test();
			}
		}
	}
}

void chunk_filter_cluster_doors(chunk& c, int threshold)
{
	for (room& r : c.rooms)
	{
		for (room& r2 : c.rooms)
		{
			int side = room_neighbours(r, r2);
			if (side >= 0 && r.isolation > threshold && r.cluster != r2.cluster)
			{
				switch (side)
				{
				case DIR_RIGHT: if (r.right == -1) { r.right = r2.left = c.roll(std::max(r.y1, r2.y1), std::min(r.y2, r2.y2)); c.build(r.x2 + 1, r.right, TILE_DOOR); } break;
				case DIR_LEFT: if (r.left == -1) { r.left = r2.right = c.roll(std::max(r.y1, r2.y1), std::min(r.y2, r2.y2)); c.build(r.x1 - 1, r.left, TILE_DOOR); } break;
				case DIR_UP: if (r.top == -1) { r.top = r2.bottom = c.roll(std::max(r.x1, r2.x1), std::min(r.x2, r2.x2)); c.build(r.top, r.y1 - 1, TILE_DOOR); } break;
				case DIR_DOWN: if (r.bottom == -1) { r.bottom = r2.top = c.roll(std::max(r.x1, r2.x1), std::min(r.x2, r2.x2)); c.build(r.bottom, r.y2 + 1, TILE_DOOR); } break;
				default: assert(false); break;
				}
				r.self_test();
				r2.self_test();
			}
		}
	}
}

void chunk_room_expand(chunk& c, int min, int max)
{
	for (int idx = 0; idx < (int)c.rooms.size(); idx++)
	{
		const room rc = c.rooms.at(idx);
		rc.self_test();
		// check each direction: do we have space to add a room expansion in that direction?
		int ndir = c.roll(rc.y1, rc.y2);
		room rl(rc.x1 - 4, ndir - 1, rc.x1 - 2, ndir + 1, rc.isolation + 1);
		if (rl.valid() && rc.left == -1 && can_build(c, rl))
		{
			room& r = c.rooms.at(idx);
			r.left = ndir;
			rl.cluster = r.cluster;
			if (r.flags & ROOM_FLAG_NEAT) { rl.y1 = r.y1; rl.y2 = r.y2; r.flags |= ROOM_FLAG_NEAT; }
			c.dig(r.x1 - 1, r.left); // dig a corridor
			if (c.roll(0, c.config.openness * 3) == 0) c.build(r.x1 - 1, r.left, TILE_DOOR);
			chunk_room_dig(c, rl); // dig the room
			rl.right = r.left;
			chunk_room_grow_randomly(c, rl, min, max);
			rl.self_test();
			c.rooms.push_back(rl);
			if (debug) print_room(c, rl);
		}
		ndir = c.roll(rc.y1, rc.y2);
		room rr(rc.x2 + 2, ndir - 1, rc.x2 + 4, ndir + 1, rc.isolation + 1);
		if (rr.valid() && rc.right == -1 && can_build(c, rr))
		{
			room& r = c.rooms.at(idx);
			r.right = ndir;
			rr.cluster = r.cluster;
			if (r.flags & ROOM_FLAG_NEAT) { rr.y1 = r.y1; rr.y2 = r.y2; r.flags |= ROOM_FLAG_NEAT; }
			c.dig(r.x2 + 1, r.right); // dig a corridor
			if (c.roll(0, c.config.openness * 3) == 0) c.build(r.x2 + 1, r.right, TILE_DOOR);
			chunk_room_dig(c, rr); // dig the room
			rr.left = r.right;
			chunk_room_grow_randomly(c, rr, min, max);
			rr.self_test();
			c.rooms.push_back(rr);
			if (debug) print_room(c, rr);
		}
		ndir = c.roll(rc.x1, rc.x2);
		room rt(ndir - 1, rc.y1 - 4, ndir + 1, rc.y1 - 2, rc.isolation + 1);
		if (rt.valid() && rc.top == -1 && can_build(c, rt))
		{
			room& r = c.rooms.at(idx);
			r.top = ndir;
			rt.cluster = r.cluster;
			if (r.flags & ROOM_FLAG_NEAT) { rt.x1 = r.x1; rt.x2 = r.x2; r.flags |= ROOM_FLAG_NEAT; }
			c.dig(r.top, r.y1 - 1); // dig a corridor
			if (c.roll(0, c.config.openness * 3) == 0) c.build(r.top, r.y1 - 1, TILE_DOOR);
			chunk_room_dig(c, rt); // dig the room
			rt.bottom = r.top;
			chunk_room_grow_randomly(c, rt, min, max);
			rt.self_test();
			c.rooms.push_back(rt);
			if (debug) print_room(c, rt);
		}
		ndir = c.roll(rc.x1, rc.x2);
		room rb(ndir - 1, rc.y2 + 2, ndir + 1, rc.y2 + 4, rc.isolation + 1);
		if (rb.valid() && rc.bottom == -1 && can_build(c, rb))
		{
			room& r = c.rooms.at(idx);
			r.bottom = ndir;
			rb.cluster = r.cluster;
			if (r.flags & ROOM_FLAG_NEAT) { rb.x1 = r.x1; rb.x2 = r.x2; r.flags |= ROOM_FLAG_NEAT; }
			c.dig(r.bottom, r.y2 + 1); // dig a corridor
			if (c.roll(0, c.config.openness * 3) == 0) c.build(r.bottom, r.y2 + 1, TILE_DOOR);
			chunk_room_dig(c, rb); // dig the room
			rb.top = r.bottom;
			chunk_room_grow_randomly(c, rb, min, max);
			rb.self_test();
			c.rooms.push_back(rb);
			if (debug) print_room(c, rb);
		}
	}
}

bool chunk_room_in_room(chunk& c, int roomidx, int space)
{
	room& r = c.rooms.at(roomidx);
	assert(space >= 1);
	if (r.size() < 24 + space * space || r.x2 - r.x1 < 4 + space || r.y2 - r.y1 < 4 + space) return false;
	room r2(r.x1 + space + 1, r.y1 + space + 1, r.x2 - space - 1, r.y2 - space - 1, r.isolation + 1, r.flags | ROOM_FLAG_NESTED);
	r2.cluster = r.cluster;
	int side = c.roll(0, 3);
	switch (side)
	{
	case 0: r2.top = c.roll(r2.x1, r2.x2); break;
	case 1: r2.right = c.roll(r2.y1, r2.y2); break;
	case 2: r2.bottom = c.roll(r2.x1, r2.x2); break;
	case 3: r2.left = c.roll(r2.y1, r2.y2); break;
	default: abort();
	}
	chunk_room_draw(c, r2, c.roll(0, c.config.openness) == 0);
	r2.self_test();
	c.rooms.push_back(r2);
	return true;
}

bool chunk_room_corners(chunk& c, int roomidx, int corners, int min)
{
	room& r = c.rooms.at(roomidx);
	bool retval = false;
	assert(min >= 9);
	int side = -1;
	if (r.flags & ROOM_FLAG_NEAT) side = c.roll(0, 1); // set for all here for more symmetry
	if (r.top == -1 && r.bottom == -1) r.top = r.x1 + (r.x2 - r.x1) / 2;
	if (r.left == -1 && r.right == -1) r.left = r.y1 + (r.y2 - r.y1) / 2;
	if (r.left == -1 && r.right != -1) r.left = r.right;
	if (r.right == -1 && r.left != -1) r.right = r.left;
	if (r.bottom == -1 && r.top != -1) r.bottom = r.top;
	if (r.top == -1 && r.bottom != -1) r.top = r.bottom;
	const short midx = r.x1 + ((r.x2 - r.x1) >> 1);
	const short midy = r.y1 + ((r.y2 - r.y1) >> 1);
	if ((corners & CHUNK_TOP_LEFT) && r.top >= 2 && r.left > 2)
	{
		room r2(r.x1, r.y1, std::min(midx, r.top) - 2, std::min(midy, r.left) - 2, r.isolation + 1, r.flags | ROOM_FLAG_NESTED);
		r2.cluster = r.cluster;
		if (r2.valid() && r2.size() >= min)
		{
			if (side == -1) side = c.roll(0, 1);
			if (side == 0) r2.right = c.roll(r2.y1, r2.y2);
			else r2.bottom = c.roll(r2.x1, r2.x2);
			chunk_room_draw(c, r2, c.roll(0, c.config.openness) == 0);
			r2.self_test();
			c.rooms.push_back(r2);
			retval = true;
		}
	}
	if ((corners & CHUNK_TOP_RIGHT) && r.top > 0 && r.right >= 2 && r.top < c.width - 1)
	{
		room r2(std::max(midx, r.top) + 2, r.y1, r.x2, std::min(midy, r.right) - 2, r.isolation + 1, r.flags | ROOM_FLAG_NESTED);
		r2.cluster = r.cluster;
		if (r2.valid() && r2.size() >= min)
		{
			if (side == -1) side = c.roll(0, 1);
			if (side == 0) r2.left = c.roll(r2.y1, r2.y2);
			else r2.bottom = c.roll(r2.x1, r2.x2);
			chunk_room_draw(c, r2, c.roll(0, c.config.openness) == 0);
			r2.self_test();
			c.rooms.push_back(r2);
			retval = true;
		}
	}
	if ((corners & CHUNK_BOTTOM_LEFT) && r.bottom >= 2 && r.left > 0 && r.left < c.height - 1)
	{
		room r2(r.x1, std::max(midy, r.left) + 2, std::min(midx, r.bottom) - 2, r.y2, r.isolation + 1, r.flags | ROOM_FLAG_NESTED);
		r2.cluster = r.cluster;
		if (r2.valid() && r2.size() >= min)
		{
			if (side == -1) side = c.roll(0, 1);
			if (side == 0) r2.right = c.roll(r2.y1, r2.y2);
			else r2.top = c.roll(r2.x1, r2.x2);
			chunk_room_draw(c, r2, c.roll(0, c.config.openness) == 0);
			r2.self_test();
			c.rooms.push_back(r2);
			retval = true;
		}
	}
	if ((corners & CHUNK_BOTTOM_RIGHT) && r.bottom > 0 && r.right > 0 && r.bottom < c.width - 1 && r.right < c.height - 1)
	{
		room r2(std::max(midx, r.bottom) + 2, std::max(midy, r.right) + 2, r.x2, r.y2, r.isolation + 1, r.flags | ROOM_FLAG_NESTED);
		r2.cluster = r.cluster;
		if (r2.valid() && r2.size() >= min)
		{
			if (side == -1) side = c.roll(0, 1);
			if (side == 0) r2.left = c.roll(r2.y1, r2.y2);
			else r2.top = c.roll(r2.x1, r2.x2);
			chunk_room_draw(c, r2, c.roll(0, c.config.openness) == 0);
			r2.self_test();
			c.rooms.push_back(r2);
			retval = true;
		}
	}
	return retval;
}