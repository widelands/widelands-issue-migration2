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

#include "logic/map_objects/immovable_program.h"

#include "logic/game.h"
#include "logic/game_data_error.h"
#include "logic/map_objects/terrain_affinity.h"
#include "logic/map_objects/world/world.h"
#include "logic/mapfringeregion.h"
#include "logic/player.h"
#include "sound/note_sound.h"

namespace Widelands {

ImmovableProgram::ImmovableProgram(const std::string& init_name,
                                   const std::vector<std::string>& lines,
                                   ImmovableDescr* immovable)
   : name_(init_name) {
	for (const std::string& line : lines) {
		try {
			ProgramParseInput parseinput = parse_program_string(line);

			// NOCOM they all have the same signature. Template?

			Action* action;
			if (parseinput.name == "animate") {
				action = new ActAnimate(parseinput.arguments, *immovable);
			} else if (parseinput.name == "transform") {
				action = new ActTransform(parseinput.arguments, *immovable);
			} else if (parseinput.name == "grow") {
				action = new ActGrow(parseinput.arguments, *immovable);
			} else if (parseinput.name == "remove") {
				action = new ActRemove(parseinput.arguments, *immovable);
			} else if (parseinput.name == "seed") {
				action = new ActSeed(parseinput.arguments, *immovable);
			} else if (parseinput.name == "playsound") {
				action = new ActPlaySound(parseinput.arguments, *immovable);
			} else if (parseinput.name == "construct") {
				action = new ActConstruct(parseinput.arguments, *immovable);
			} else {
				throw GameDataError("unknown command type \"%s\"", parseinput.name.c_str());
			}
			actions_.push_back(action);
		} catch (const GameDataError& e) {
			throw GameDataError("Error parsing line\"%s\": %s", line.c_str(), e.what());
		}
	}
	if (actions_.empty())
		throw GameDataError("no actions");
}

ImmovableProgram::Action::~Action() {
}

ImmovableProgram::ActAnimate::ActAnimate(const std::vector<std::string>& arguments, ImmovableDescr& descr) {
	parameters = parse_act_animate(arguments, descr, true);
}

/// Use convolutuion to make the animation time a random variable with binomial
/// distribution and the configured time as the expected value.
void ImmovableProgram::ActAnimate::execute(Game& game, Immovable& immovable) const {
	immovable.start_animation(game, parameters.animation);
	immovable.program_step(
	   game, parameters.duration ? 1 + game.logic_rand() % parameters.duration + game.logic_rand() % parameters.duration : 0);
}

ImmovableProgram::ActPlaySound::ActPlaySound(const std::vector<std::string>& arguments, const ImmovableDescr& descr) {
	parameters = parse_act_play_sound(arguments, descr, 127);
}

/** Demand from the g_sound_handler to play a certain sound effect.
 * Whether the effect actually gets played
 * is decided only by the sound server*/
void ImmovableProgram::ActPlaySound::execute(Game& game, Immovable& immovable) const {
	Notifications::publish(NoteSound(parameters.name, immovable.get_position(), parameters.priority));
	immovable.program_step(game);
}

ImmovableProgram::ActTransform::ActTransform(std::vector<std::string>& arguments, ImmovableDescr& descr) {
	try {
		tribe = true;
		bob = false;
		probability = 0;

		for (uint32_t i = 0; i < arguments.size(); ++i) {
			if (arguments[i] == "bob")
				bob = true;
			else if (arguments[i] == "immovable")
				bob = false;
			else if (arguments[i][0] >= '0' && arguments[i][0] <= '9') {
				probability = read_positive(arguments[i], 254);
			} else {
				std::vector<std::string> segments = split_string(arguments[i], ":");

				if (segments.size() > 2)
					throw GameDataError("object type has more than 2 segments");
				if (segments.size() == 2) {
					if (segments[0] == "world")
						tribe = false;
					else if (segments[0] == "tribe") {
						if (descr.owner_type() != MapObjectDescr::OwnerType::kTribe)
							throw GameDataError("scope \"tribe\" does not match the immovable type");
						tribe = true;
					} else
						throw GameDataError("unknown scope \"%s\" given for target type (must be "
						                    "\"world\" or \"tribe\")",
						                    segments[0].c_str());

					type_name = segments[1];
				} else {
					type_name = segments[0];
				}
			}
		}
		if (type_name == descr.name())
			throw GameDataError("illegal transformation to the same type");
	} catch (const WException& e) {
		throw GameDataError("transform: %s", e.what());
	}
}

void ImmovableProgram::ActTransform::execute(Game& game, Immovable& immovable) const {
	if (probability == 0 || game.logic_rand() % 256 < probability) {
		Player* player = immovable.get_owner();
		Coords const c = immovable.get_position();
		MapObjectDescr::OwnerType owner_type = immovable.descr().owner_type();
		immovable.remove(game);  //  Now immovable is a dangling reference!

		if (bob) {
			game.create_ship(c, type_name, player);
		} else {
			game.create_immovable_with_name(
			   c, type_name, owner_type, player, nullptr /* former_building_descr */);
		}
	} else
		immovable.program_step(game);
}

ImmovableProgram::ActGrow::ActGrow(std::vector<std::string>& arguments, ImmovableDescr& descr) {
	if (arguments.size() != 1) {
		throw GameDataError("Usage: grow=<immovable name>");
	}
	if (!descr.has_terrain_affinity()) {
		throw GameDataError(
		   "Immovable %s can 'grow', but has no terrain_affinity entry.", descr.name().c_str());
	}

	// NOCOM check if the target immovable exists
	type_name = arguments.front();
}

void ImmovableProgram::ActGrow::execute(Game& game, Immovable& immovable) const {
	const Map& map = game.map();
	FCoords const f = map.get_fcoords(immovable.get_position());
	const ImmovableDescr& descr = immovable.descr();

	if ((game.logic_rand() % TerrainAffinity::kPrecisionFactor) <
	    probability_to_grow(descr.terrain_affinity(), f, map, game.world().terrains())) {
		MapObjectDescr::OwnerType owner_type = descr.owner_type();
		Player* owner = immovable.get_owner();
		immovable.remove(game);  //  Now immovable is a dangling reference!
		game.create_immovable_with_name(
		   f, type_name, owner_type, owner, nullptr /* former_building_descr */);
	} else {
		immovable.program_step(game);
	}
}

/**
 * remove
 */
ImmovableProgram::ActRemove::ActRemove(std::vector<std::string>& arguments, ImmovableDescr&) {
	if (arguments.size() > 1) {
		throw GameDataError("Usage: remove=[probability]");
	}
	probability = arguments.empty() ? 0 : read_positive(arguments.front(), 254);
}

void ImmovableProgram::ActRemove::execute(Game& game, Immovable& immovable) const {
	if (probability == 0 || game.logic_rand() % 256 < probability)
		immovable.remove(game);  //  Now immovable is a dangling reference!
	else
		immovable.program_step(game);
}

ImmovableProgram::ActSeed::ActSeed(std::vector<std::string>& arguments, ImmovableDescr& descr) {
	// NOCOM code duplication with ActGrow
	if (arguments.size() != 1) {
		throw GameDataError("Usage: seed=<immovable name>");
	}
	if (!descr.has_terrain_affinity()) {
		throw GameDataError(
		   "Immovable %s can 'seed', but has no terrain_affinity entry.", descr.name().c_str());
	}

	// NOCOM check if the target immovable exists
	type_name = arguments.front();
}

void ImmovableProgram::ActSeed::execute(Game& game, Immovable& immovable) const {
	const Map& map = game.map();
	FCoords const f = map.get_fcoords(immovable.get_position());
	const ImmovableDescr& descr = immovable.descr();

	if ((game.logic_rand() % TerrainAffinity::kPrecisionFactor) <
	    probability_to_grow(descr.terrain_affinity(), f, map, game.world().terrains())) {
		// Seed a new tree.
		MapFringeRegion<> mr(map, Area<>(f, 0));
		uint32_t fringe_size = 0;
		do {
			mr.extend(map);
			fringe_size += 6;
		} while (game.logic_rand() % std::numeric_limits<uint8_t>::max() < probability);

		for (uint32_t n = game.logic_rand() % fringe_size; n; --n) {
			mr.advance(map);
		}

		const FCoords new_location = map.get_fcoords(mr.location());
		if (!new_location.field->get_immovable() &&
		    (new_location.field->nodecaps() & MOVECAPS_WALK) &&
		    (game.logic_rand() % TerrainAffinity::kPrecisionFactor) <
		       probability_to_grow(
		          descr.terrain_affinity(), new_location, map, game.world().terrains())) {
			game.create_immovable_with_name(mr.location(), type_name, descr.owner_type(),
			                                nullptr /* owner */, nullptr /* former_building_descr */);
		}
	}

	immovable.program_step(game);
}

ImmovableProgram::ActConstruct::ActConstruct(std::vector<std::string>& arguments, ImmovableDescr& descr) {
	// NOCOM signature: construct=idle 5000 210000
	if (arguments.size() != 3) {
		throw GameDataError("Usage: construct=<animation> <build duration> <decay duration>");
	}
	try {
		const std::string animation_name = arguments[0];
		if (!descr.is_animation_known(animation_name)) {
			throw GameDataError("Unknown animation \"%s\" in immovable program for immovable \"%s\"",
			                    animation_name.c_str(), descr.name().c_str());
		}
		animid_ = descr.get_animation(animation_name);

		buildtime_ = read_positive(arguments[1]);
		decaytime_ = read_positive(arguments[2]);
	} catch (const WException& e) {
		throw GameDataError("construct: %s", e.what());
	}
}

constexpr uint8_t kCurrentPacketVersionConstructionData = 1;


const char* ActConstructData::name() const {
	return "construct";
}
void ActConstructData::save(FileWrite& fw, Immovable& imm) const {
	fw.unsigned_8(kCurrentPacketVersionConstructionData);
	delivered.save(fw, imm.get_owner()->tribe());
}

ActConstructData* ActConstructData::load(FileRead& fr, Immovable& imm) {
	ActConstructData* d = new ActConstructData;

	try {
		uint8_t packet_version = fr.unsigned_8();
		if (packet_version == kCurrentPacketVersionConstructionData) {
			d->delivered.load(fr, imm.get_owner()->tribe());
		} else {
			throw UnhandledVersionError(
			   "ActConstructData", packet_version, kCurrentPacketVersionConstructionData);
		}
	} catch (const WException& e) {
		delete d;
		d = nullptr;
		throw GameDataError("ActConstructData: %s", e.what());
	}

	return d;
}


void ImmovableProgram::ActConstruct::execute(Game& g, Immovable& imm) const {
	ActConstructData* d = imm.get_action_data<ActConstructData>();
	if (!d) {
		// First execution
		d = new ActConstructData;
		imm.set_action_data(d);

		imm.start_animation(g, animid_);
		imm.anim_construction_total_ = imm.descr().buildcost().total();
	} else {
		// Perhaps we are called due to the construction timeout of the last construction step
		Buildcost remaining;
		imm.construct_remaining_buildcost(g, &remaining);
		if (remaining.empty()) {
			imm.program_step(g);
			return;
		}

		// Otherwise, this is a decay timeout
		uint32_t totaldelivered = 0;
		for (Buildcost::const_iterator it = d->delivered.begin(); it != d->delivered.end(); ++it)
			totaldelivered += it->second;

		if (!totaldelivered) {
			imm.remove(g);
			return;
		}

		uint32_t randdecay = g.logic_rand() % totaldelivered;
		for (Buildcost::iterator it = d->delivered.begin(); it != d->delivered.end(); ++it) {
			if (randdecay < it->second) {
				it->second--;
				break;
			}

			randdecay -= it->second;
		}

		imm.anim_construction_done_ = d->delivered.total();
	}

	imm.program_step_ = imm.schedule_act(g, decaytime_);
}

ImmovableActionData*
ImmovableActionData::load(FileRead& fr, Immovable& imm, const std::string& name) {
	// TODO(GunChleoc): Use "construct" only after Build 20
	if (name == "construction" || name == "construct")
		return ActConstructData::load(fr, imm);
	else {
		log("ImmovableActionData::load: type %s not known", name.c_str());
		return nullptr;
	}
}
}  // namespace Widelands
