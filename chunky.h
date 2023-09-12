#pragma once

#include "dice.h"
#include <assert.h>
#include <vector>
#include <deque>
#include <stdint.h>
#include <signal.h>
#include <stdio.h>

// -- Debug --

#define DICEY_STRINGIFY(x) #x
#define DICEY_TOSTRING(x) DICEY_STRINGIFY(x)
#ifndef NDEBUG
#define CHUNK_ASSERT(c, expr) do { if (!dicey_likely(expr)) { printf("Failed %s in %s line %d (%s), printing failed chunk from seed %lu...\n", DICEY_TOSTRING(expr), __FILE__, __LINE__, __FUNCTION__, (unsigned long)(c).config.orig.orig); (c).print_chunk(); raise(SIGTRAP); } } while (false)
#else
#define CHUNK_ASSERT(c, expr)
#endif
#ifndef NDEBUG
#define ROOM_ASSERT(c, r, expr) do { if (!dicey_likely(expr)) { printf("Failed %s in %s line %d (%s), printing failed chunk from seed %lu...\n", DICEY_TOSTRING(expr), __FILE__, __LINE__, __FUNCTION__, (unsigned long)(c).config.orig.orig); print_room(c, r); raise(SIGTRAP); } } while (false)
#else
#define ROOM_ASSERT(c, expr)
#endif

// -- Constants --

#define DIR_UP 0b1000
#define DIR_RIGHT 0b0100
#define DIR_DOWN 0b0010
#define DIR_LEFT 0b0001
#define DIR_CROSS 0b1111

enum corner_type
{
	CHUNK_TOP_LEFT = 1,
	CHUNK_TOP_RIGHT = 2,
	CHUNK_BOTTOM_LEFT = 4,
	CHUNK_BOTTOM_RIGHT = 8,
};

enum
{
	ROOM_FLAG_NESTED = 1,
	ROOM_FLAG_NEAT = 2, // strive for more symmetry
	ROOM_FLAG_CORRIDOR = 4,
	ROOM_FLAG_FURNISHED = 8, // not empty anymore
};

enum tile_type
{
	TILE_ROCK,
	TILE_EMPTY,
	TILE_DOOR,
	TILE_ONE_WAY_TOP,
	TILE_ONE_WAY_BOTTOM,
	TILE_ONE_WAY_LEFT,
	TILE_ONE_WAY_RIGHT,
	TILE_WALL,
	TILE_WALL_DAMAGED,
	TILE_DEBRIS,
	TILE_RAIL,
	TILE_SENTINEL,
	TILE_TURRET,
	TILE_TOTEM,
	TILE_TRAP,
	TILE_ALTAR,
	TILE_SHRINE,
	TILE_HIDDEN_GROVE,
	TILE_SHRUB,

	ENTITY_BOSS,
	ENTITY_LEADER,
	ENTITY_SUPPORT,
	ENTITY_TANK,
	ENTITY_DAMAGE,
	ENTITY_SPECIALIST,
	ENTITY_WILD, // random wild mob
};

struct chunkconfig
{
	chunkconfig(seed s) : state(s), orig(s) {}
	int width = 32;
	int height = 32;
	/// The lower this value, the more likely to have doors and other obstacles.
	int openness = 2;
	/// The higher this value, the more chaotic the design.
	int chaos = 1;
	/// The higher this value, the more broken down everything will be. Make sure changing this does not alter the layout.
	int brokenness = 1;
	seed state;
	seed orig;

	// Our place in the world
	int x = 0;
	int y = 0;
	int level_width = 32;
	int level_height = 32;
};

/// Room dimensions are equal to its dug out inner space, not including the walls surrounding it.
struct room
{
	int16_t x1;
	int16_t y1;
	int16_t x2;
	int16_t y2;

	int16_t top = -1;
	int16_t bottom = -1;
	int16_t left = -1;
	int16_t right = -1;

	int8_t isolation = 0;
	int8_t flags = 0;

	void self_test() const;
	int size() const { return (x2 - x1 + 1) * (y2 - y1 + 1); }
	bool valid() const { return (x2 >= x1 && y2 >= y1 && x2 >= 0 && y2 >= 0 && y1 >= 0 && x1 >= 0 && (top <= x2 || top == -1) && (top >= x1 || top == -1) && (bottom <= x2 || bottom == -1) && (bottom >= x1 || bottom == -1) && (left >= y1 || left == -1) && (left <= y2 || left == -1) && (right >= y1 || right == -1) && (right <= y2 || right == -1)); }
	inline int width() const { return x2 - x1; }
	inline int height() const { return y2 - y1; }
	inline int door_count() const { return (top != -1) + (bottom != -1) + (left != -1) + (right != -1); }

	room(int _x1, int _y1, int _x2, int _y2, int _isolation = 0, int _flags = 0) : x1(_x1), y1(_y1), x2(_x2), y2(_y2), isolation(_isolation), flags(_flags) {}
};
inline bool operator==(const room& lhs, const room& rhs){ return (lhs.x1 == rhs.x1 && lhs.y1 == rhs.y1 && lhs.x2 == rhs.x2 && lhs.y2 == rhs.y2); }
inline bool operator!=(const room& lhs, const room& rhs){ return !(lhs == rhs); }

/// A small chunk of a level's map. Limitations: It must be power-of-two size in each direction. And it can only have
/// one exit to the next chunk in each direction (not including stairs, portals, etc.).
struct chunk
{
	chunk(const chunkconfig& c);

	inline void consider_door(int x, int y) { if (config.state.roll(0, config.openness * 2) == 0) build(x, y, TILE_DOOR); else build(x, y, TILE_EMPTY); }
	inline void make_exit_top(int v) { top = v; dig(v, 0); consider_door(v, 0); }
	inline void make_exit_left(int v) { left = v; dig(0, v); consider_door(0, v); }
	inline void make_exit_bottom(int v) { bottom = v; dig(v, config.height - 1); consider_door(v, config.height - 1); }
	inline void make_exit_right(int v) { right = v; dig(config.width - 1, v); consider_door(config.width - 1, v); }

	/// Deterministically generate exits for a chunk without having generated your neighbours yet. You need to give the (x, y) positions
	/// of the chunk and the total size of the map, both in terms of chunks.
	void generate_exits()
	{
		if (config.y > 0 && top == -1) { make_exit_top(config.state.derive(config.x, config.y - 1, 0).roll(3, config.width - 3)); } // Top
		if (config.x > 0 && left == -1) { make_exit_left(config.state.derive(config.x - 1, config.y, 1).roll(3, config.height - 3)); } // Left
		if (config.y < config.level_height - 1 && bottom == -1) { make_exit_bottom(config.state.derive(config.x, config.y, 0).roll(3, config.width - 3)); } // Bottom
		if (config.x < config.level_width - 1 && right == -1) { make_exit_right(config.state.derive(config.x, config.y, 1).roll(3, config.height - 3)); } // Right
	}

	/// Excavate a room. The space must be filled with rocks only.
	void dig_room(const room& r)
	{
		for (int x = r.x1; x <= r.x2; x++) { for (int y = r.y1; y <= r.y2; y++) { CHUNK_ASSERT(*this, rock(x, y)); map[(y << bits) + x] = TILE_EMPTY; } }
		for (int x = r.x1 - 1; x <= r.x2 + 1; x++) { if (rock(x, r.y1 - 1)) makewall(x, r.y1 - 1); if (rock(x, r.y2 + 1)) makewall(x, r.y2 + 1); }
		for (int y = r.y1 - 1; y <= r.y2 + 1; y++) { if (rock(r.x1 - 1, y)) makewall(r.x1 - 1, y); if (rock(r.x2 + 1, y)) makewall(r.x2 + 1, y); }
		if (r.top > 0) consider_door(r.top, r.y1 - 1);
		if (r.bottom > 0) consider_door(r.bottom, r.y2 + 1);
		if (r.left > 0) consider_door(r.x1 - 1, r.left);
		if (r.right > 0) consider_door(r.x2 + 1, r.right);
	}

	/// Add a room inside another room. The space must be empty.
	void add_room(const room& r, int sides = DIR_CROSS)
	{
		for (int x = r.x1; x <= r.x2; x++) { for (int y = r.y1; y <= r.y2; y++) { CHUNK_ASSERT(*this, empty(x, y)); } }
		for (int x = r.x1 - 1; x <= r.x2 + 1; x++) { if ((sides & DIR_UP) && empty(x, r.y1 - 1)) makewall(x, r.y1 - 1); if ((sides & DIR_DOWN) && empty(x, r.y2 + 1)) makewall(x, r.y2 + 1); }
		for (int y = r.y1 - 1; y <= r.y2 + 1; y++) { if ((sides & DIR_LEFT) && empty(r.x1 - 1, y)) makewall(r.x1 - 1, y); if ((sides & DIR_RIGHT) && empty(r.x2 + 1, y)) makewall(r.x2 + 1, y); }
		if (r.top > 0) consider_door(r.top, r.y1 - 1);
		if (r.bottom > 0) consider_door(r.bottom, r.y2 + 1);
		if (r.left > 0) consider_door(r.x1 - 1, r.left);
		if (r.right > 0) consider_door(r.x2 + 1, r.right);
	}

	// Low-level functions
	inline bool rock(int x, int y) const { const int i = map.at((y << bits) + x); return i == TILE_ROCK || i == TILE_WALL || i == TILE_WALL_DAMAGED; }
	inline bool empty(int x, int y) const { return (map[(y << bits) + x] == TILE_EMPTY); }
	inline bool wall(int x, int y) const { const int i = map[(y << bits) + x]; return i == TILE_WALL || i == TILE_WALL_DAMAGED; }
	inline void fill(int x, int y) { map[(y << bits) + x] = TILE_ROCK; }
	inline void makewall(int x, int y) { map[(y << bits) + x] = TILE_WALL; }
	inline uint8_t at(int x, int y) const { return map.at((y << bits) + x); }
	inline bool border(int x, int y) const { return (x == 0 || y == 0 || x == width - 1 || y == height -1); }
	inline void build(int x, int y, tile_type t) { map[(y << bits) + x] = t; }
	inline bool try_build(int x, int y, tile_type t) { if (empty(x, y)) { map[(y << bits) + x] = t; return true; } else return false; }
	inline void dig(int x, int y) { map[(y << bits) + x] = TILE_EMPTY; for (int i = std::max(0, x - 1); i <= std::min(width - 1, x + 1); i++) for (int j = std::max(0, y - 1); j <= std::min(height - 1, y + 1); j++) if (rock(i, j)) map[(j << bits) + i] = TILE_WALL; }
	inline int roll(int low, int high) { return config.state.roll(low, high); } // convenience function
	inline void horizontal_corridor(int x1, int x2, int y) { for (int i = x1; i <= x2; i++) { dig(i, y); } rooms.emplace_back(x1, y, x2, y, ROOM_FLAG_CORRIDOR); }
	inline void vertical_corridor(int x, int y1, int y2) { for (int i = y1; i <= y2; i++) { dig(x, i); } rooms.emplace_back(x, y1, x, y2, ROOM_FLAG_CORRIDOR); }
	void beautify();
	inline int room_index(room& r) const { int i = 0; for (const room& rr : rooms) { if (r == rr) return i; i++; } return -1; }

	int16_t width;
	int16_t height;

	// Exits
	int top = -1;
	int bottom = -1;
	int left = -1;
	int right = -1;

	void self_test() const;
	void room_list_self_test() const;
	void print_chunk() const;

	chunkconfig config; // TBD some duplication here

	std::deque<room> rooms;

private:
	unsigned bits; // number of bits to bitshift to move from row to row
	std::vector<uint8_t> map;
};

// -- Filters --

/// Simple filter that tries to connects the exits by digging tunnels to them, stopping at the first open space. Assumes exits are
/// already dug out. Returns built corridors as rooms in a room list.
void chunk_filter_connect_exits(chunk& c);

// Specialized connecting algorithms ...
bool chunk_filter_connect_exits_inner_loop(chunk& c); // ... using an inner loop rectangle.
bool chunk_filter_connect_exits_grand_central(chunk& c); // ... using a large center room.

/// Filter that tries to fill larger rooms with other rooms
void chunk_filter_room_in_room(chunk& c);

/// Filter that adds one-way doors to reduce unfun backtracking.
void chunk_filter_one_way_doors(chunk& c, int threshold = 3);

/// Expand every room into more rooms where possible, with min/max parameters for room expansion size
/// and neat flag if you want rooms to align more neatly.
void chunk_filter_room_expand(chunk& c, int min = 2, int max = 6);

/// Find the best room in the chunk for a boss room, and place a boss token in it.
room& chunk_filter_boss_placement(chunk& c, int flags);

// -- Room utility functions --

/// Fill a room.
void chunk_room_dig(chunk& c, const room& r);

/// Draw a room. Walls will be drawn around *outside* the given room coordinates.
void chunk_room_draw(chunk& c, const room& r, bool door);

/// Shrink room by 2 tiles from the top
bool chunk_room_shrink_top(chunk& c, room& r);

/// Shrink room by 2 tiles from the bottom
bool chunk_room_shrink_bottom(chunk& c, room& r);

/// Shrink room by 2 tiles from the right
bool chunk_room_shrink_right(chunk& c, room& r);

/// Shrink room by 2 tiles from the left
bool chunk_room_shrink_left(chunk& c, room& r);

/// Try to expand the given room in all directions; returns true if successful.
bool chunk_room_grow(chunk& c, room& r);

/// Grow in random directions.
void chunk_room_grow_randomly(chunk& c, room& r, int min, int max);

/// Try to add a corner room. 'corners' is a bitfield telling which corners to try. 'min' is minimum size to allow.
/// Will *not* check if you already have a room-in-a-room in this room!
bool chunk_room_corners(chunk& c, room& r, int corners, int min = 9);

/// Try to add a room inside a room.
bool chunk_room_in_room(chunk& c, room& r, int space = 1);

/// Put mobs in surrounding rooms.
bool chunk_filter_protect_room(chunk& c, room& r);

/// Put random wild mobs in every empty room.
bool chunk_filter_wildlife(chunk& c);

// -- Debug functions --

void print_room(const chunk& c, int roomidx);
void print_room(const chunk& c, const room& r);
