/*
 * Copyright (C) 2007-2020 by the Widelands Development Team
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

#include "map_io/map_players_view_packet.h"

#include <boost/algorithm/string.hpp>

#include "base/log.h"
#include "base/wexception.h"
#include "economy/flag.h"
#include "economy/road.h"
#include "io/fileread.h"
#include "io/filewrite.h"
#include "logic/editor_game_base.h"
#include "logic/field.h"
#include "logic/game_data_error.h"
#include "logic/map_objects/descriptions.h"
#include "logic/map_objects/tribes/tribe_descr.h"
#include "logic/player.h"

namespace Widelands {

constexpr uint16_t kCurrentPacketVersion = 4;

inline bool from_unsigned(unsigned value) {
	return value == 1;
}

void MapPlayersViewPacket::read(FileSystem& fs,
                                EditorGameBase& egbase,
                                const WorldLegacyLookupTable& world_lookup_table,
                                const TribesLegacyLookupTable& tribes_lookup_table) {
	FileRead fr;
	if (!fr.try_open(fs, "binary/view")) {
		// TODO(Nordfriese): Savegame compatibility – require this packet after v1.0
		log_warn("New-style view packet not found. There may be strange effects regarding unseen "
		         "areas.\n");
		return;
	}

	try {
		uint16_t const packet_version = fr.unsigned_16();
		const Map& map = egbase.map();
		if (packet_version >= 3 && packet_version <= kCurrentPacketVersion) {
			const PlayerNumber nr_players = fr.unsigned_8();
			if (map.get_nrplayers() != nr_players) {
				throw wexception("Wrong number of players. Expected %d but read %d from packet\n",
				                 static_cast<unsigned>(map.get_nrplayers()),
				                 static_cast<unsigned>(nr_players));
			}
			MapIndex no_of_fields = map.max_index();

			iterate_players_existing(p, nr_players, egbase, player) {
				const unsigned player_no_from_packet = fr.unsigned_8();
				if (p != player_no_from_packet) {
					throw wexception("Wrong player number. Expected %d but read %d from packet\n",
					                 static_cast<unsigned>(p),
					                 static_cast<unsigned>(player_no_from_packet));
				}

				std::set<Player::Field*> seen_fields;

				player->revealed_fields_.clear();
				const unsigned no_revealed_fields = fr.unsigned_32();
				for (unsigned i = 0; i < no_revealed_fields; ++i) {
					const MapIndex revealed_index = fr.unsigned_32();
					player->revealed_fields_.insert(revealed_index);
					player->rediscover_node(map, map.get_fcoords(map[revealed_index]));
				}

				// Read numerical field infos as combined strings to reduce number of hard disk write
				// operations

				// Some data for all player fields
				// While saving, we iterated all fields for a property and separated the fields by '|'.
				std::vector<std::string> field_vector;

				// Single complex record for a field
				// If a property is multidimensional (e.g. each field has 3 roads), the values are
				// separated by '*'.
				std::vector<std::string> data_vector;
				std::string parseme;

				// Seeing
				parseme = fr.c_string();
				boost::split(field_vector, parseme, boost::is_any_of("|"));
				assert(field_vector.size() == no_of_fields);

				for (MapIndex m = 0; m < no_of_fields; ++m) {
					Player::Field& f = player->fields_[m];
					f.seeing = static_cast<Widelands::SeeUnseeNode>(stoi(field_vector[m]));
					if (f.seeing == SeeUnseeNode::kPreviouslySeen) {
						seen_fields.insert(&f);
					}
				}

				size_t no_of_seen_fields = fr.unsigned_32();

				// Skip data for fields that were never seen, e.g. this happens during saveloading a
				// backup while starting a new game
				if (no_of_seen_fields == 0) {
					continue;
				}

				// TODO(Nordfriese): Savegame compatibility
				if (packet_version >= 4) {
					const size_t additionally_seen = fr.unsigned_32();
					no_of_seen_fields += additionally_seen;
					if (additionally_seen > 0) {
						parseme = fr.c_string();
						boost::split(field_vector, parseme, boost::is_any_of("|"));
						assert(field_vector.size() == additionally_seen);
						for (size_t i = 0; i < additionally_seen; ++i) {
							Player::Field& f = player->fields_[stoi(field_vector[i])];
							assert(f.seeing == SeeUnseeNode::kUnexplored);
							seen_fields.insert(&f);
						}
					}
				}

				if (seen_fields.size() != no_of_seen_fields) {
					throw wexception("Read %" PRIuS
					                 " unseen fields but detected %d when the packet was written\n",
					                 seen_fields.size(), static_cast<unsigned>(no_of_seen_fields));
				}

				// Owner: playernumber|playernumber|playernumber ...
				parseme = fr.c_string();
				boost::split(field_vector, parseme, boost::is_any_of("|"));
				assert(field_vector.size() == seen_fields.size());

				size_t counter = 0;
				for (auto& field : seen_fields) {
					field->owner = stoi(field_vector[counter]);
					++counter;
				}
				assert(counter == no_of_seen_fields);

				// Last Unseen: time|time|time ...
				parseme = fr.c_string();
				boost::split(field_vector, parseme, boost::is_any_of("|"));
				assert(field_vector.size() == no_of_seen_fields);

				counter = 0;
				for (auto& field : seen_fields) {
					field->time_node_last_unseen = Time(stol(field_vector[counter]));
					++counter;
				}
				assert(counter == no_of_seen_fields);

				// Last Surveyed: time|time|time ...
				parseme = fr.c_string();
				boost::split(field_vector, parseme, boost::is_any_of("|"));
				assert(field_vector.size() == no_of_seen_fields);

				counter = 0;
				for (auto& field : seen_fields) {
					boost::split(data_vector, field_vector[counter], boost::is_any_of("*"));
					assert(data_vector.size() == 2);

					field->time_triangle_last_surveyed[0] = Time(stol(data_vector[0]));
					field->time_triangle_last_surveyed[1] = Time(stol(data_vector[1]));
					++counter;
				}
				assert(counter == no_of_seen_fields);

				// Resource Amounts: down*right|down*right|down*right| ...
				parseme = fr.c_string();
				boost::split(field_vector, parseme, boost::is_any_of("|"));
				assert(field_vector.size() == no_of_seen_fields);

				counter = 0;
				for (auto& field : seen_fields) {
					boost::split(data_vector, field_vector[counter], boost::is_any_of("*"));
					assert(data_vector.size() == 2);

					field->resource_amounts.d = stoi(data_vector[0]);
					field->resource_amounts.r = stoi(data_vector[1]);
					++counter;
				}
				assert(counter == no_of_seen_fields);

				// Terrains: down*right|down*right|down*right| ...
				parseme = fr.c_string();
				boost::split(field_vector, parseme, boost::is_any_of("|"));
				assert(field_vector.size() == no_of_seen_fields);

				counter = 0;
				for (auto& field : seen_fields) {
					boost::split(data_vector, field_vector[counter], boost::is_any_of("*"));
					assert(data_vector.size() == 2);

					field->terrains.d = stoi(data_vector[0]);
					field->terrains.r = stoi(data_vector[1]);
					++counter;
				}
				assert(counter == no_of_seen_fields);

				// Roads east*southeast*southwest|east*southeast*southwest| ...
				parseme = fr.c_string();
				boost::split(field_vector, parseme, boost::is_any_of("|"));
				assert(field_vector.size() == no_of_seen_fields);

				counter = 0;
				for (auto& field : seen_fields) {
					boost::split(data_vector, field_vector[counter], boost::is_any_of("*"));
					assert(data_vector.size() == 3);

					field->r_e = static_cast<Widelands::RoadSegment>(stoi(data_vector[0]));
					field->r_se = static_cast<Widelands::RoadSegment>(stoi(data_vector[1]));
					field->r_sw = static_cast<Widelands::RoadSegment>(stoi(data_vector[2]));
					++counter;
				}
				assert(counter == no_of_seen_fields);

				// Borders: here*right*bottom_right*bottom_left|here*right*bottom_right*bottom_left| ...
				parseme = fr.c_string();
				boost::split(field_vector, parseme, boost::is_any_of("|"));
				assert(field_vector.size() == no_of_seen_fields);

				counter = 0;
				for (auto& field : seen_fields) {
					boost::split(data_vector, field_vector[counter], boost::is_any_of("*"));
					assert(data_vector.size() == 4);

					field->border = from_unsigned(stoi(data_vector[0]));
					field->border_r = from_unsigned(stoi(data_vector[1]));
					field->border_br = from_unsigned(stoi(data_vector[2]));
					field->border_bl = from_unsigned(stoi(data_vector[3]));
					++counter;
				}
				assert(counter == no_of_seen_fields);

				// Map objects
				for (auto& field : seen_fields) {
					std::string descr = fr.string();
					if (descr.empty()) {
						field->map_object_descr = nullptr;
					} else {
						// I here assume that no two immovables will have the same internal name
						if (descr == "flag") {
							field->map_object_descr = &g_flag_descr;
						} else if (descr == "portdock") {
							field->map_object_descr = &g_portdock_descr;
						} else {
							DescriptionIndex di = egbase.descriptions().building_index(
							   tribes_lookup_table.lookup_building(descr));
							if (di != INVALID_INDEX) {
								field->map_object_descr = egbase.descriptions().get_building_descr(di);
							} else {
								di = egbase.descriptions().immovable_index(
								   tribes_lookup_table.lookup_immovable(
								      world_lookup_table.lookup_immovable(descr)));
								if (di != INVALID_INDEX) {
									field->map_object_descr = egbase.descriptions().get_immovable_descr(di);
								} else {
									throw GameDataError("invalid map_object_descr: %s", descr.c_str());
								}
							}
						}

						if (field->map_object_descr->type() == MapObjectType::DISMANTLESITE) {
							field->partially_finished_building.dismantlesite.building =
							   egbase.descriptions().get_building_descr(
							      egbase.descriptions().safe_building_index(
							         tribes_lookup_table.lookup_building(fr.string())));
							field->partially_finished_building.dismantlesite.progress = fr.unsigned_32();
						} else if (field->map_object_descr->type() == MapObjectType::CONSTRUCTIONSITE) {
							field->partially_finished_building.constructionsite.becomes =
							   egbase.descriptions().get_building_descr(
							      egbase.descriptions().safe_building_index(
							         tribes_lookup_table.lookup_building(fr.string())));
							descr = fr.string();
							field->partially_finished_building.constructionsite.was =
							   descr.empty() ? nullptr :
							                   egbase.descriptions().get_building_descr(
							                      egbase.descriptions().safe_building_index(
							                         tribes_lookup_table.lookup_building(descr)));

							for (uint32_t j = fr.unsigned_32(); j; --j) {
								field->partially_finished_building.constructionsite.intermediates.push_back(
								   egbase.descriptions().get_building_descr(
								      egbase.descriptions().safe_building_index(
								         tribes_lookup_table.lookup_building(fr.string()))));
							}

							field->partially_finished_building.constructionsite.totaltime = Duration(fr);
							field->partially_finished_building.constructionsite.completedtime =
							   Duration(fr);
						}
					}
				}
			}
		} else if (packet_version >= 1 && packet_version <= 2) {
			// TODO(Nordfriese): Savegame compatibility, remove after v1.0
			for (uint8_t i = fr.unsigned_8(); i; --i) {
				Player& player = *egbase.get_player(fr.unsigned_8());

				player.revealed_fields_.clear();
				for (uint32_t j = fr.unsigned_32(); j; --j) {
					player.revealed_fields_.insert(fr.unsigned_32());
				}

				for (MapIndex m = map.max_index(); m; --m) {
					Player::Field& f = player.fields_[m - 1];

					f.owner = fr.unsigned_8();

					f.seeing = static_cast<SeeUnseeNode>(fr.unsigned_8());

					if (f.seeing != SeeUnseeNode::kPreviouslySeen && packet_version > 1) {
						continue;
					}

					f.time_node_last_unseen = Time(fr);
					f.time_triangle_last_surveyed[0] = Time(fr);
					f.time_triangle_last_surveyed[1] = Time(fr);

					f.resource_amounts.d = fr.unsigned_8();
					f.resource_amounts.r = fr.unsigned_8();

					f.terrains.d = fr.unsigned_32();
					f.terrains.r = fr.unsigned_32();

					f.r_e = static_cast<RoadSegment>(fr.unsigned_8());
					f.r_se = static_cast<RoadSegment>(fr.unsigned_8());
					f.r_sw = static_cast<RoadSegment>(fr.unsigned_8());

					f.border = fr.unsigned_8();
					f.border_r = fr.unsigned_8();
					f.border_br = fr.unsigned_8();
					f.border_bl = fr.unsigned_8();

					std::string descr = fr.string();
					if (descr.empty()) {
						f.map_object_descr = nullptr;
					} else {
						// I here assume that no two immovables will have the same internal name
						if (descr == "flag") {
							f.map_object_descr = &g_flag_descr;
						} else if (descr == "portdock") {
							f.map_object_descr = &g_portdock_descr;
						} else {
							DescriptionIndex di = egbase.descriptions().building_index(descr);
							if (di != INVALID_INDEX) {
								f.map_object_descr = egbase.descriptions().get_building_descr(di);
							} else {
								di = egbase.descriptions().immovable_index(descr);
								if (di != INVALID_INDEX) {
									f.map_object_descr = egbase.descriptions().get_immovable_descr(di);
								} else {
									throw GameDataError("invalid map_object_descr: %s", descr.c_str());
								}
							}
						}

						if (packet_version > 1) {
							if (f.map_object_descr->type() == MapObjectType::DISMANTLESITE) {
								f.partially_finished_building.dismantlesite.building =
								   egbase.descriptions().get_building_descr(
								      egbase.descriptions().safe_building_index(fr.string()));
								f.partially_finished_building.dismantlesite.progress = fr.unsigned_32();
							} else if (f.map_object_descr->type() == MapObjectType::CONSTRUCTIONSITE) {
								f.partially_finished_building.constructionsite.becomes =
								   egbase.descriptions().get_building_descr(
								      egbase.descriptions().safe_building_index(fr.string()));
								descr = fr.string();
								f.partially_finished_building.constructionsite.was =
								   descr.empty() ? nullptr :
								                   egbase.descriptions().get_building_descr(
								                      egbase.descriptions().safe_building_index(descr));

								for (uint32_t j = fr.unsigned_32(); j; --j) {
									f.partially_finished_building.constructionsite.intermediates.push_back(
									   egbase.descriptions().get_building_descr(
									      egbase.descriptions().safe_building_index(fr.string())));
								}

								f.partially_finished_building.constructionsite.totaltime = Duration(fr);
								f.partially_finished_building.constructionsite.completedtime = Duration(fr);
							}
						} else {
							descr = fr.string();
							if (descr.empty()) {
								f.partially_finished_building.dismantlesite.building = nullptr;
								f.partially_finished_building.dismantlesite.progress = 0;
							} else {
								f.partially_finished_building.constructionsite.becomes =
								   egbase.descriptions().get_building_descr(
								      egbase.descriptions().safe_building_index(descr));

								descr = fr.string();
								f.partially_finished_building.constructionsite.was =
								   descr.empty() ? nullptr :
								                   egbase.descriptions().get_building_descr(
								                      egbase.descriptions().safe_building_index(descr));

								for (uint32_t j = fr.unsigned_32(); j; --j) {
									f.partially_finished_building.constructionsite.intermediates.push_back(
									   egbase.descriptions().get_building_descr(
									      egbase.descriptions().safe_building_index(fr.string())));
								}

								f.partially_finished_building.constructionsite.totaltime = Duration(fr);
								f.partially_finished_building.constructionsite.completedtime = Duration(fr);
							}
						}
					}
				}
			}
		} else {
			throw UnhandledVersionError("MapPlayersViewPacket", packet_version, kCurrentPacketVersion);
		}
	} catch (const WException& e) {
		throw GameDataError("view: %s", e.what());
	}
}

inline unsigned to_unsigned(bool value) {
	return value ? 1 : 0;
}

void MapPlayersViewPacket::write(FileSystem& fs, EditorGameBase& egbase) {
	FileWrite fw;

	fw.unsigned_16(kCurrentPacketVersion);

	const Map& map = egbase.map();
	const PlayerNumber nr_players = map.get_nrplayers();
	fw.unsigned_8(nr_players);

	iterate_players_existing(p, nr_players, egbase, player) {
		fw.unsigned_8(p);
		std::set<const Player::Field*> seen_fields;
		std::set<const Player::Field*> additionally_seen_fields;
		// Explicitly revealed fields
		fw.unsigned_32(player->revealed_fields_.size());
		for (const MapIndex& m : player->revealed_fields_) {
			fw.unsigned_32(m);
		}

		// Write numerical field infos as combined strings to reduce number of hard disk write
		// operations
		{
			// Seeing
			std::ostringstream oss("");
			const MapIndex upper_bound = map.max_index() - 1;
			for (MapIndex m = 0; m < upper_bound; ++m) {
				const Player::Field& f = player->fields_[m];
				oss << static_cast<unsigned>(f.seeing) << "|";
				if (f.seeing == SeeUnseeNode::kPreviouslySeen) {
					seen_fields.insert(&f);
					// The data for some of the terrains and edges between PreviouslySeen
					// and Unexplored fields is stored in an Unexplored field. The data
					// for this field therefore needs to be saveloaded as well.
					const Coords coords(m % map.get_width(), m / map.get_width());
					for (const Coords& c : {map.tr_n(coords), map.tl_n(coords), map.l_n(coords)}) {
						const Player::Field& neighbour = player->fields_[map.get_index(c)];
						if (neighbour.seeing == SeeUnseeNode::kUnexplored) {
							additionally_seen_fields.insert(&neighbour);
						}
					}
				}
			}
			const Player::Field& f = player->fields_[upper_bound];
			oss << static_cast<unsigned>(f.seeing);
			if (f.seeing == SeeUnseeNode::kPreviouslySeen) {
				seen_fields.insert(&f);
			}
			fw.c_string(oss.str());
		}

		// Write number of seen fields
		fw.unsigned_32(seen_fields.size());

		// Skip data for fields that were never seen
		if (seen_fields.empty()) {
			assert(additionally_seen_fields.empty());
			continue;
		}

		fw.unsigned_32(additionally_seen_fields.size());
		if (!additionally_seen_fields.empty()) {
			std::ostringstream oss("");
			for (auto it = additionally_seen_fields.begin(); it != additionally_seen_fields.end();) {
				seen_fields.insert(*it);
				const MapIndex index = (*it) - &player->fields_[0];
				oss << index;
				++it;
				if (it != additionally_seen_fields.end()) {
					oss << "|";
				}
			}
			fw.c_string(oss.str());
		}

		{
			// Owner
			std::ostringstream oss("");
			for (auto it = seen_fields.begin(); it != seen_fields.end();) {
				oss << static_cast<unsigned>((*it)->owner);
				++it;
				if (it != seen_fields.end()) {
					oss << "|";
				}
			}
			fw.c_string(oss.str());
		}
		{
			// Last Unseen
			std::ostringstream oss("");
			for (auto it = seen_fields.begin(); it != seen_fields.end();) {
				oss << static_cast<unsigned>((*it)->time_node_last_unseen.get());
				++it;
				if (it != seen_fields.end()) {
					oss << "|";
				}
			}
			fw.c_string(oss.str());
		}
		{
			// Last Surveyed
			std::ostringstream oss("");
			for (auto it = seen_fields.begin(); it != seen_fields.end();) {
				oss << (*it)->time_triangle_last_surveyed[0].get() << "*"
				    << (*it)->time_triangle_last_surveyed[1].get();
				++it;
				if (it != seen_fields.end()) {
					oss << "|";
				}
			}
			fw.c_string(oss.str());
		}
		{
			// Resource Amounts
			std::ostringstream oss("");
			for (auto it = seen_fields.begin(); it != seen_fields.end();) {
				oss << static_cast<unsigned>((*it)->resource_amounts.d) << "*"
				    << static_cast<unsigned>((*it)->resource_amounts.r);
				++it;
				if (it != seen_fields.end()) {
					oss << "|";
				}
			}
			fw.c_string(oss.str());
		}
		{
			// Terrains
			std::ostringstream oss("");
			for (auto it = seen_fields.begin(); it != seen_fields.end();) {
				oss << (*it)->terrains.d << "*" << (*it)->terrains.r;
				++it;
				if (it != seen_fields.end()) {
					oss << "|";
				}
			}
			fw.c_string(oss.str());
		}
		{
			// Roads
			std::ostringstream oss("");
			for (auto it = seen_fields.begin(); it != seen_fields.end();) {
				oss << static_cast<unsigned>((*it)->r_e) << "*" << static_cast<unsigned>((*it)->r_se)
				    << "*" << static_cast<unsigned>((*it)->r_sw);
				++it;
				if (it != seen_fields.end()) {
					oss << "|";
				}
			}
			fw.c_string(oss.str());
		}
		{
			// Borders
			std::ostringstream oss("");
			for (auto it = seen_fields.begin(); it != seen_fields.end();) {
				oss << to_unsigned((*it)->border) << "*" << to_unsigned((*it)->border_r) << "*"
				    << to_unsigned((*it)->border_br) << "*" << to_unsigned((*it)->border_bl);
				++it;
				if (it != seen_fields.end()) {
					oss << "|";
				}
			}
			fw.c_string(oss.str());
		}

		// Map objects
		for (const auto& field : seen_fields) {
			if (field->map_object_descr) {
				fw.string(field->map_object_descr->name());

				if (field->map_object_descr->type() == MapObjectType::DISMANTLESITE) {
					// `building` can only be nullptr in compatibility cases.
					// Remove the non-null check after v1.0
					fw.string(field->partially_finished_building.dismantlesite.building ?
					             field->partially_finished_building.dismantlesite.building->name() :
					             "dismantlesite");
					fw.unsigned_32(field->partially_finished_building.dismantlesite.progress);
				} else if (field->map_object_descr->type() == MapObjectType::CONSTRUCTIONSITE) {
					fw.string(field->partially_finished_building.constructionsite.becomes->name());
					fw.string(field->partially_finished_building.constructionsite.was ?
					             field->partially_finished_building.constructionsite.was->name() :
					             "");

					fw.unsigned_32(
					   field->partially_finished_building.constructionsite.intermediates.size());
					for (const BuildingDescr* d :
					     field->partially_finished_building.constructionsite.intermediates) {
						fw.string(d->name());
					}

					field->partially_finished_building.constructionsite.totaltime.save(fw);
					field->partially_finished_building.constructionsite.completedtime.save(fw);
				}
			} else {
				fw.string("");
			}
		}
	}

	fw.write(fs, "binary/view");
}
}  // namespace Widelands
