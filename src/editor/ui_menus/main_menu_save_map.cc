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

#include "editor/ui_menus/main_menu_save_map.h"

#include <cstdio>
#include <cstring>
#include <memory>
#include <string>

#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>

#include "base/i18n.h"
#include "base/macros.h"
#include "base/wexception.h"
#include "editor/editorinteractive.h"
#include "editor/ui_menus/main_menu_map_options.h"
#include "editor/ui_menus/main_menu_save_map_make_directory.h"
#include "io/filesystem/filesystem.h"
#include "io/filesystem/layered_filesystem.h"
#include "io/filesystem/zip_filesystem.h"
#include "logic/filesystem_constants.h"
#include "logic/generic_save_handler.h"
#include "map_io/map_saver.h"
#include "map_io/widelands_map_loader.h"
#include "ui_basic/messagebox.h"
#include "wui/mapdetails.h"
#include "wui/maptable.h"

inline EditorInteractive& MainMenuSaveMap::eia() {
	return dynamic_cast<EditorInteractive&>(*get_parent());
}

// TODO(GunChleoc): Arabic: Make directory dialog: buttons need more height for Arabic.
MainMenuSaveMap::MainMenuSaveMap(EditorInteractive& parent)
   : MainMenuLoadOrSaveMap(parent, 3, "save_map_menu", _("Save Map"), "maps/My_Maps"),

     make_directory_(this,
                     "make_directory",
                     right_column_x_,
                     tabley_ + tableh_ + 3 * padding_ - 1,
                     get_inner_w() - right_column_x_ - padding_,
                     buth_,
                     UI::ButtonStyle::kWuiSecondary,
                     _("Make Directory")),
     edit_options_(this,
                   "edit_options",
                   right_column_x_,
                   tabley_ + tableh_ - buth_,
                   get_inner_w() - right_column_x_ - padding_,
                   buth_,
                   UI::ButtonStyle::kWuiPrimary,
                   _("Map Options")),
     editbox_label_(this, padding_, tabley_ + tableh_ + 3 * padding_, butw_, buth_, _("Filename:")),
     illegal_filename_tooltip_(FileSystem::illegal_filename_tooltip()) {
	set_current_directory(curdir_);

	// Make room for edit_options_ button
	map_details_.set_size(map_details_.get_w(), map_details_.get_h() - buth_ - padding_);

	table_.selected.connect(boost::bind(&MainMenuSaveMap::clicked_item, boost::ref(*this)));
	table_.double_clicked.connect(
	   boost::bind(&MainMenuSaveMap::double_clicked_item, boost::ref(*this)));
	table_.cancel.connect(boost::bind(&MainMenuSaveMap::die, this));
	table_.set_can_focus(true);

	editbox_ = new UI::EditBox(
	   this, editbox_label_.get_x() + editbox_label_.get_w() + padding_, editbox_label_.get_y(),
	   tablew_ - editbox_label_.get_w() - padding_ + 1, buth_, 2, UI::PanelStyle::kWui);
	editbox_->set_text(parent.egbase().map().get_name());

	editbox_->changed.connect(boost::bind(&MainMenuSaveMap::edit_box_changed, this));
	edit_box_changed();
	editbox_->ok.connect(boost::bind(&MainMenuSaveMap::clicked_ok, boost::ref(*this)));
	editbox_->cancel.connect(boost::bind(
	   &MainMenuSaveMap::reset_editbox_or_die, boost::ref(*this), parent.egbase().map().get_name()));

	ok_.sigclicked.connect(boost::bind(&MainMenuSaveMap::clicked_ok, boost::ref(*this)));
	cancel_.sigclicked.connect(boost::bind(&MainMenuSaveMap::die, boost::ref(*this)));

	make_directory_.sigclicked.connect(
	   boost::bind(&MainMenuSaveMap::clicked_make_directory, boost::ref(*this)));
	edit_options_.sigclicked.connect(
	   boost::bind(&MainMenuSaveMap::clicked_edit_options, boost::ref(*this)));

	// We always want the current map's data here
	const Widelands::Map& map = parent.egbase().map();
	MapData::MapType maptype;

	if (map.scenario_types() & Widelands::Map::MP_SCENARIO ||
	    map.scenario_types() & Widelands::Map::SP_SCENARIO) {
		maptype = MapData::MapType::kScenario;
	} else {
		maptype = MapData::MapType::kNormal;
	}

	MapData mapdata(map, "", maptype, MapData::DisplayType::kMapnames);

	map_details_.update(mapdata, false);
}

/**
 * Called when the ok button was pressed or a file in list was double clicked.
 */
void MainMenuSaveMap::clicked_ok() {
	if (!ok_.enabled()) {
		return;
	}
	std::string filename = editbox_->text();
	std::string complete_filename;

	if (filename == "" && table_.has_selection()) {  //  Maybe a directory is selected.
		complete_filename = filename = maps_data_[table_.get_selected()].filename;
	} else {
		complete_filename = curdir_ + g_fs->file_separator() + filename;
	}

	if (g_fs->is_directory(complete_filename.c_str()) &&
	    !Widelands::WidelandsMapLoader::is_widelands_map(complete_filename)) {
		set_current_directory(complete_filename);
		fill_table();
	} else {  //  Ok, save this map
		Widelands::Map* map = eia().egbase().mutable_map();
		if (map->get_name() == _("No Name")) {
			std::string::size_type const filename_size = filename.size();
			map->set_name(4 <= filename_size && boost::iends_with(filename, kWidelandsMapExtension) ?
			                 filename.substr(0, filename_size - 4) :
			                 filename);
		}
		if (save_map(filename, !g_options.pull_section("global").get_bool("nozip", false))) {
			die();
		} else {
			table_.focus();
		}
	}
}

/**
 * Called, when the make directory button was clicked.
 */
void MainMenuSaveMap::clicked_make_directory() {
	/** TRANSLATORS: A folder that hasn't been given a name yet */
	MainMenuSaveMapMakeDirectory md(this, _("unnamed"));
	bool open_dialogue = true;
	while (open_dialogue) {
		open_dialogue = false;
		if (md.run<UI::Panel::Returncodes>() == UI::Panel::Returncodes::kOk) {
			std::string fullname = curdir_ + g_fs->file_separator() + md.get_dirname();
			// Trim it for preceding/trailing whitespaces in user input
			boost::trim(fullname);
			if (g_fs->file_exists(fullname)) {
				const std::string s = _("A file or directory with that name already exists.");
				UI::WLMessageBox mbox(
				   this, _("Error Creating Directory!"), s, UI::WLMessageBox::MBoxType::kOk);
				mbox.run<UI::Panel::Returncodes>();
				open_dialogue = true;
			} else {
				try {
					g_fs->ensure_directory_exists(curdir_);
					//  Create directory.
					g_fs->make_directory(fullname);
				} catch (const FileError& e) {
					log("directory creation failed in MainMenuSaveMap::"
					    "clicked_make_directory: %s\n",
					    e.what());
					const std::string s =
					   (boost::format(_("Error while creating directory ‘%s’.")) % fullname).str();
					UI::WLMessageBox mbox(
					   this, _("Error Creating Directory!"), s, UI::WLMessageBox::MBoxType::kOk);
					mbox.run<UI::Panel::Returncodes>();
				}
				fill_table();
			}
		}
	}
	table_.focus();
	// TODO(Arty): In case of successful dir creation we should select the
	// new dir in the table.
}

void MainMenuSaveMap::clicked_edit_options() {
	MainMenuMapOptions mo(eia(), true);
	if (mo.run<UI::Panel::Returncodes>() == UI::Panel::Returncodes::kOk) {
		const Widelands::Map& map = eia().egbase().map();
		MapData::MapType maptype;

		if (map.scenario_types() & Widelands::Map::MP_SCENARIO ||
		    map.scenario_types() & Widelands::Map::SP_SCENARIO) {
			maptype = MapData::MapType::kScenario;
		} else {
			maptype = MapData::MapType::kNormal;
		}

		MapData mapdata(map, editbox_->text(), maptype, MapData::DisplayType::kMapnames);

		map_details_.update(mapdata, false);
	}
}

/**
 * called when an item was selected
 */
void MainMenuSaveMap::clicked_item() {
	// Only change editbox contents
	if (table_.has_selection()) {
		const MapData& mapdata = maps_data_[table_.get_selected()];
		if (mapdata.maptype != MapData::MapType::kDirectory) {
			editbox_->set_text(
			   FileSystem::fs_filename(maps_data_[table_.get_selected()].filename.c_str()));
			edit_box_changed();
		}
	}
}

/**
 * An Item has been doubleclicked
 */
void MainMenuSaveMap::double_clicked_item() {
	assert(table_.has_selection());
	const MapData& mapdata = maps_data_[table_.get_selected()];
	if (mapdata.maptype == MapData::MapType::kDirectory) {
		set_current_directory(mapdata.filename);
		fill_table();
	} else {
		clicked_ok();
	}
	edit_box_changed();
}

/**
 * The editbox was changed. Enable ok button
 */
void MainMenuSaveMap::edit_box_changed() {
	// Prevent the user from creating nonsense file names, like e.g. ".." or "...".
	const bool is_legal_filename = FileSystem::is_legal_filename(editbox_->text());
	ok_.set_enabled(is_legal_filename);
	editbox_->set_tooltip(is_legal_filename ? "" : illegal_filename_tooltip_);
}

void MainMenuSaveMap::reset_editbox_or_die(const std::string& current_filename) {
	if (editbox_->text() == current_filename) {
		die();
	} else {
		editbox_->set_text(current_filename);
	}
}

void MainMenuSaveMap::set_current_directory(const std::string& filename) {
	curdir_ = filename;
	directory_info_.set_text(
	   /** TRANSLATORS: The folder that a file will be saved to. */
	   (boost::format(_("Current directory: %s")) % (_("My Maps") + curdir_.substr(basedir_.size())))
	      .str());
}

/**
 * Save the map in the current directory with
 * the given filename
 *
 * returns true if dialog should close, false if it
 * should stay open
 */
bool MainMenuSaveMap::save_map(std::string filename, bool binary) {
	// Trim it for preceding/trailing whitespaces in user input
	boost::trim(filename);

	//  OK, first check if the extension matches (ignoring case).
	if (!boost::iends_with(filename, kWidelandsMapExtension)) {
		filename += kWidelandsMapExtension;
	}

	//  Append directory name.
	const std::string complete_filename = curdir_ + g_fs->file_separator() + filename;

	//  Check if file exists. If so, show a warning.
	if (g_fs->file_exists(complete_filename)) {
		const std::string s =
		   (boost::format(_("A file with the name ‘%s’ already exists. Overwrite?")) %
		    FileSystem::fs_filename(filename.c_str()))
		      .str();
		UI::WLMessageBox mbox(this, _("Error Saving Map!"), s, UI::WLMessageBox::MBoxType::kOkCancel);
		if (mbox.run<UI::Panel::Returncodes>() == UI::Panel::Returncodes::kBack) {
			return false;
		}
	}

	Widelands::EditorGameBase& egbase = eia().egbase();
	Widelands::Map* map = egbase.mutable_map();

	// Recompute seafaring tag
	map->cleanup_port_spaces(egbase.world());
	if (map->allows_seafaring()) {
		map->add_tag("seafaring");
	} else {
		map->delete_tag("seafaring");
	}

	if (map->has_artifacts()) {
		map->add_tag("artifacts");
	} else {
		map->delete_tag("artifacts");
	}

	// Try saving the map.
	GenericSaveHandler gsh(
	   [&egbase](FileSystem& fs) {
		   Widelands::MapSaver wms(fs, egbase);
		   wms.save();
	   },
	   complete_filename, binary ? FileSystem::ZIP : FileSystem::DIR);
	GenericSaveHandler::Error error = gsh.save();

	// If only the temporary backup couldn't be deleted, we still treat it as
	// success. Automatic cleanup will deal with later. No need to bother the
	// player with it.
	if (error == GenericSaveHandler::Error::kSuccess ||
	    error == GenericSaveHandler::Error::kDeletingBackupFailed) {
		egbase.get_ibase()->log_message(_("Map saved"));
		return true;
	}

	std::string msg = gsh.localized_formatted_result_message();
	UI::WLMessageBox mbox(this, _("Error Saving Map!"), msg, UI::WLMessageBox::MBoxType::kOk);
	mbox.run<UI::Panel::Returncodes>();

	// If only the backup failed (likely just because of a file lock),
	// then leave the dialog open for the player to try with a new filename.
	if (error == GenericSaveHandler::Error::kBackupFailed) {
		return false;
	}

	// In the other error cases close the dialog.
	return true;
}
