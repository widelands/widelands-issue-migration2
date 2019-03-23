/*
 * Copyright (C) 2002-2019 by the Widelands Development Team
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

#include "logic/map_objects/map_object_program.h"

#include "io/filesystem/layered_filesystem.h"
#include "logic/game_data_error.h"
#include "logic/map_objects/map_object.h"
#include "sound/sound_handler.h"

namespace Widelands {

MapObjectProgram::MapObjectProgram(const std::string& init_name) : name_ (init_name) {}

std::string MapObjectProgram::name() const {
	return name_;
}

std::vector<std::string> MapObjectProgram::split_string(const std::string& s, const char* const separators) {
	std::vector<std::string> result;
	for (std::string::size_type pos = 0, endpos;
	     (pos = s.find_first_not_of(separators, pos)) != std::string::npos; pos = endpos) {
		endpos = s.find_first_of(separators, pos);
		result.push_back(s.substr(pos, endpos - pos));
	}
	return result;
}


unsigned int MapObjectProgram::read_number(const std::string& input, int min_value, int max_value) {
	unsigned int  result = 0U;
	char* endp;
	long int const value = strtol(input.c_str(), &endp, 0);
	result = value;
	if (*endp || result != value) {
		throw GameDataError("Expected a number but found \"%s\"", input.c_str());
	}
	if (value < min_value) {
		throw GameDataError("Expected a number >= %d but found \"%s\"", min_value, input.c_str());
	}
	if (value > max_value) {
		throw GameDataError("Expected a number <= %d but found \"%s\"", max_value, input.c_str());
	}
	return result;
}

unsigned int MapObjectProgram::read_positive(const std::string& input, int max_value) {
	return read_number(input, 1, max_value);
}

MapObjectProgram::ProgramParseInput MapObjectProgram::parse_program_string(const std::string& line) {
	const std::pair<std::string, std::string> key_values = MapObjectProgram::read_key_value_pair(line, '=');
	return ProgramParseInput{key_values.first, split_string(key_values.second, " \t\n")};
}

const std::pair<std::string, std::string> MapObjectProgram::read_key_value_pair(const std::string& input, const char separator, const std::string& default_value, const std::string& expected_key) {
	const size_t idx = input.find(separator);
	const std::string key = input.substr(0, idx);

	if (!expected_key.empty()) {
		if (idx == input.npos) {
			throw GameDataError("Empty value in '%s' for separator '%c'\n", input.c_str(), separator);
		}
		if (key != expected_key) {
			throw GameDataError("Expected key '%s' but found '%s' in '%s'\n", expected_key.c_str(), key.c_str(), input.c_str());
		}
	}

	return std::make_pair(key, idx == input.npos ? default_value : input.substr(idx + 1));
}

MapObjectProgram::AnimationParameters MapObjectProgram::parse_act_animate(const std::vector<std::string>& arguments, const MapObjectDescr& descr, bool is_idle_allowed) {
	AnimationParameters result;
	if (arguments.size() < 1) {
		throw GameDataError("%s: Animation without name. Usage: animate=<name> <duration>",
							descr.name().c_str());
	}

	const std::string animation_name = arguments.at(0);

	if (arguments.size() > 2) {
		throw GameDataError("%s: Animation %s has too many parameters. Usage: animate=<name> <duration>",
							descr.name().c_str(), animation_name.c_str());
	}
	if (!is_idle_allowed && animation_name == "idle") {
		throw GameDataError("%s: 'idle' animation is default; calling is not allowed",
							descr.name().c_str());
	}
	if (!descr.is_animation_known(animation_name)) {
		throw GameDataError("%s: Unknown animation '%s'",
							descr.name().c_str(), animation_name.c_str());
	}
	result.animation = descr.get_animation(animation_name);

	if (arguments.size() == 2) {  //  The next parameter is the duration.
		result.duration = read_positive(arguments.at(1));
	}
	return result;
}

MapObjectProgram::PlaySoundParameters MapObjectProgram::parse_act_play_sound(const std::vector<std::string>& arguments, const MapObjectDescr& descr, uint8_t default_priority) {
	PlaySoundParameters result;

	if (arguments.size() < 2 || arguments.size() > 3) {
		throw GameDataError("%s: Wrong number of parameters. Usage: playsound <sound_dir> <sound_name> [priority]", descr.name().c_str());
	}

	result.name = arguments.at(0)+ g_fs->file_separator() + arguments.at(1);

	g_sound_handler.load_fx_if_needed(arguments.at(0), arguments.at(1), result.name);

	result.priority = arguments.size() == 3 ? read_positive(arguments.at(2)) : default_priority;
	return result;
}

}  // namespace Widelands
