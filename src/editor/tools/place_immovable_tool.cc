/*
 * Copyright (C) 2002-2022 by the Widelands Development Team
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "editor/tools/place_immovable_tool.h"

#include "base/macros.h"
#include "editor/editorinteractive.h"
#include "logic/editor_game_base.h"
#include "logic/field.h"
#include "logic/map_objects/immovable.h"
#include "logic/mapregion.h"

/**
 * Choses an object to place randomly from all enabled
 * and places this on the current field
 */
int32_t EditorPlaceImmovableTool::handle_click_impl(const Widelands::NodeAndTriangle<>& center,
						    EditorActionArgs* args,
						    Widelands::Map* map) {
	const int32_t radius = args->sel_radius;
	if (get_nr_enabled() == 0) {
		return radius;
	}
	Widelands::EditorGameBase& egbase = parent_.egbase();
	if (args->old_immovable_types.empty()) {
		Widelands::MapRegion<Widelands::Area<Widelands::FCoords>> mr(
		   *map, Widelands::Area<Widelands::FCoords>(map->get_fcoords(center.node), radius));
		do {
			const Widelands::BaseImmovable* im = mr.location().field->get_immovable();
			args->old_immovable_types.push_back((im != nullptr ? im->descr().name() : ""));
			args->new_immovable_types.push_back(get_random_enabled());
		} while (mr.advance(*map));
	}

	if (!args->new_immovable_types.empty()) {
		Widelands::MapRegion<Widelands::Area<Widelands::FCoords>> mr(
		   *map, Widelands::Area<Widelands::FCoords>(map->get_fcoords(center.node), radius));
		std::list<Widelands::DescriptionIndex>::iterator i = args->new_immovable_types.begin();
		do {
			if ((mr.location().field->get_immovable() == nullptr) &&
			    ((mr.location().field->nodecaps() & Widelands::MOVECAPS_WALK) != 0)) {
				egbase.create_immovable(mr.location(), *i, nullptr /* owner */);
			}
			++i;
		} while (mr.advance(*map));
	}
	return radius + 2;
}

int32_t EditorPlaceImmovableTool::handle_undo_impl(
   const Widelands::NodeAndTriangle<Widelands::Coords>& center,
   EditorActionArgs* args,
   Widelands::Map* map) {
	const int32_t radius = args->sel_radius;
	if (args->old_immovable_types.empty()) {
		return radius;
	}

	Widelands::EditorGameBase& egbase = parent_.egbase();
	Widelands::MapRegion<Widelands::Area<Widelands::FCoords>> mr(
	   *map, Widelands::Area<Widelands::FCoords>(map->get_fcoords(center.node), radius));
	std::list<std::string>::iterator i = args->old_immovable_types.begin();
	do {
		if (upcast(Widelands::Immovable, immovable, mr.location().field->get_immovable())) {
			immovable->remove(egbase);
		}
		if (!i->empty()) {
			egbase.create_immovable_with_name(
			   mr.location(), *i, nullptr /* owner */, nullptr /* former_building_descr */);
		}
		++i;
	} while (mr.advance(*map));
	return radius + 2;
}

EditorActionArgs EditorPlaceImmovableTool::format_args_impl() {
	return EditorTool::format_args_impl();
}



std::string EditorPlaceImmovableTool::format_conf_description_impl(const ToolConf& conf) {
	const Widelands::Descriptions& descriptions = parent_.egbase().descriptions();
	const Widelands::DescriptionMaintainer<Widelands::ImmovableDescr>& immovable_descriptions
           = descriptions.immovables();

	std::string mapobj_names;

        for (Widelands::DescriptionIndex idx: conf.map_obj_types) {
                if (!mapobj_names.empty()) {
                        mapobj_names += " | ";
                }
                mapobj_names += immovable_descriptions.get(idx).descname();
	}

        /** TRANSLATORS: An entry in the tool history list. */
	return format(_("Immovable: %1$s"), mapobj_names);
}

bool EditorPlaceImmovableTool::save_configuration_impl(ToolConf& conf) {
	int j = get_nr_enabled();

	if (j == 0) {
		return false;
	}

	for (int i = 0; j > 0; ++i) {
		if (is_enabled(i)) {
			conf.map_obj_types.push_back(i);
			--j;
		}
	}

	return true;
}


void EditorPlaceImmovableTool::load_configuration(const ToolConf& conf) {
	disable_all();
	for (Widelands::DescriptionIndex idx : conf.map_obj_types) {
		enable(idx, true);
	}
}
