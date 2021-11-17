/*
 * Copyright (C) 2004-2021 by the Widelands Development Team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#ifndef WL_BASE_RANDOM_H
#define WL_BASE_RANDOM_H

#include <cassert>
#include <string>

extern const uint32_t rng_sbox[256];

class StreamRead;
class StreamWrite;

struct RNG {
	RNG();
	explicit RNG(const uint32_t s) : RNG() {
		seed(s);
	}

	void seed(uint32_t);

	uint32_t rand();

	void read_state(StreamRead&);
	void write_state(StreamWrite&);

	static uint32_t static_rand();
	static inline uint32_t static_rand(const uint32_t exclusive_upper_bound) {
		assert(exclusive_upper_bound > 1);
		return static_rand() % exclusive_upper_bound;
	}

private:
	uint32_t state0;
	uint32_t state1;
};

#define SIMPLE_RAND(x) (((x) >> 8) ^ rng_sbox[(x)&0xff])

/// Generates a random UUID, looking like "550e8400-e29b-11d4-a716-446655440000".
/// This does not use logic_rand(), but instead a thread local random number
/// generator, so do not use in logic code - it will desync.
std::string generate_random_uuid();

#endif  // end of include guard: WL_BASE_RANDOM_H
