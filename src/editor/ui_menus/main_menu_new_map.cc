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

#include "editor/ui_menus/main_menu_new_map.h"

#include <memory>
#include <string>
#include <vector>

#include <boost/format.hpp>

#include "base/i18n.h"
#include "base/macros.h"
#include "editor/editorinteractive.h"
#include "graphic/font_handler.h"
#include "graphic/image.h"
#include "graphic/texture.h"
#include "logic/editor_game_base.h"
#include "logic/map.h"
#include "logic/map_objects/world/terrain_description.h"
#include "logic/map_objects/world/world.h"
#include "ui_basic/progresswindow.h"

inline EditorInteractive& MainMenuNewMap::eia() {
	return dynamic_cast<EditorInteractive&>(*get_parent());
}

MainMenuNewMap::MainMenuNewMap(EditorInteractive& parent)
   : UI::Window(&parent, "new_map_menu", 0, 0, 360, 150, _("New Map")),
     margin_(4),
     box_width_(get_inner_w() - 2 * margin_),
     box_(this, margin_, margin_, UI::Box::Vertical, 0, 0, margin_),
     width_(&box_,
            0,
            0,
            box_width_,
            box_width_ / 3,
            24,
            _("Width"),
            UI::DropdownType::kTextual,
            UI::PanelStyle::kWui),
     height_(&box_,
             0,
             0,
             box_width_,
             box_width_ / 3,
             24,
             _("Height"),
             UI::DropdownType::kTextual,
             UI::PanelStyle::kWui),
     list_(&box_, 0, 0, box_width_, 330, UI::PanelStyle::kWui),
     // Buttons
     button_box_(&box_, 0, 0, UI::Box::Horizontal, 0, 0, margin_),
     ok_button_(&button_box_,
                "create_map",
                0,
                0,
                box_width_ / 2 - margin_,
                0,
                UI::ButtonStyle::kWuiPrimary,
                _("Create Map")),
     cancel_button_(&button_box_,
                    "generate_map",
                    0,
                    0,
                    box_width_ / 2 - margin_,
                    0,
                    UI::ButtonStyle::kWuiSecondary,
                    _("Cancel")) {

	for (const int32_t& i : Widelands::kMapDimensions) {
		width_.add(std::to_string(i), i);
		height_.add(std::to_string(i), i);
	}
	width_.select(parent.egbase().map().get_width());
	height_.select(parent.egbase().map().get_height());
	width_.set_max_items(12);
	height_.set_max_items(12);

	box_.set_size(100, 20);  // Prevent assert failures
	box_.add(&width_);
	box_.add(&height_);
	box_.add_space(margin_);
	UI::Textarea* terrain_label = new UI::Textarea(&box_, _("Terrain:"));
	box_.add(terrain_label);
	box_.add(&list_);
	box_.add_space(2 * margin_);

	cancel_button_.sigclicked.connect(boost::bind(&MainMenuNewMap::clicked_cancel, this));
	ok_button_.sigclicked.connect(boost::bind(&MainMenuNewMap::clicked_create_map, this));
	if (UI::g_fh->fontset()->is_rtl()) {
		button_box_.add(&ok_button_);
		button_box_.add(&cancel_button_);
	} else {
		button_box_.add(&cancel_button_);
		button_box_.add(&ok_button_);
	}
	box_.add(&button_box_);

	box_.set_size(box_width_, width_.get_h() + height_.get_h() + terrain_label->get_h() +
	                             list_.get_h() + button_box_.get_h() + 9 * margin_);
	set_size(get_w(), box_.get_h() + 2 * margin_ + get_h() - get_inner_h());
	fill_list();
	center_to_parent();
}

void MainMenuNewMap::clicked_create_map() {
	EditorInteractive& parent = eia();
	Widelands::EditorGameBase& egbase = parent.egbase();
	Widelands::Map* map = egbase.mutable_map();
	UI::ProgressWindow loader_ui;

	loader_ui.step(_("Creating empty map…"));

	parent.cleanup_for_load();

	map->create_empty_map(
	   egbase.world(),
	   width_.get_selected() > 0 ? width_.get_selected() : Widelands::kMapDimensions[0],
	   height_.get_selected() > 0 ? height_.get_selected() : Widelands::kMapDimensions[0],
	   list_.get_selected(), _("No Name"),
	   g_options.pull_section("global").get_string("realname", pgettext("author_name", "Unknown")));

	egbase.create_tempfile_and_save_mapdata(FileSystem::ZIP);

	map->recalc_whole_map(egbase.world());
	parent.map_changed(EditorInteractive::MapWas::kReplaced);
	die();
}

void MainMenuNewMap::clicked_cancel() {
	die();
}

/*
 * fill the terrain list
 */
void MainMenuNewMap::fill_list() {
	list_.clear();
	const Widelands::DescriptionMaintainer<Widelands::TerrainDescription>& terrains =
	   eia().egbase().world().terrains();

	for (Widelands::DescriptionIndex index = 0; index < terrains.size(); ++index) {
		const Widelands::TerrainDescription& terrain = terrains.get(index);
		upcast(Image const, image, &terrain.get_texture(0));
		list_.add(terrain.descname(), index, image);
	}
	list_.select(0);
}
