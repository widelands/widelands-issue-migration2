/*
 * Copyright (C) 2020-2020 by the Widelands Development Team
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

#ifndef WL_LOGIC_ADDONS_H
#define WL_LOGIC_ADDONS_H

#include <functional>
#include <map>
#include <string>
#include <vector>

enum class AddOnCategory {
	kNone,
	kWorld,
	kTribes,
	kScript,
	kMaps,
	kCampaign,
	kWinCondition,
	kStartingCondition
};

// Note: Below you will see some lines like `std::function<std::string()> descname`.
// The reason why we implement translatable texts as functions here is that the encapsulating
// objects will stick around a long time and we don't want to have to re-create them when we
// change languages. The values implemented as one-liner functions are used only rarely so
// that this does not matter performance-wise.

struct AddOnCategoryInfo {
	std::string internal_name;
	std::function<std::string()> descname;
	std::string icon;
	bool can_disable_addons;
};

constexpr uint32_t kNotInstalled = 0;

// Required add-ons for an add-on, map, or savegame with the recommended version
using AddOnRequirements = std::vector<std::pair<std::string, uint32_t>>;

// TODO(Nordfriese): Ugly hack required for the dummy server. Can go when we have a real server.
struct AddOnFileList {
	std::vector<std::string> directories, files;
};

struct AddOnInfo {
	std::string internal_name;                 // "cool_feature.wad"
	std::function<std::string()> descname;     // "Cool Feature"
	std::function<std::string()> description;  // "This add-on is a really cool feature."
	std::string author;                        // "The Widelands Bunnybot"
	uint32_t version;                          // Add-on version
	uint32_t i18n_version;                     // (see doc/sphinx/source/add-ons.rst)
	AddOnCategory category;
	std::vector<std::string> requirements;  // This add-on will only work correctly if these
	                                        // add-ons are present in this order and active
	bool verified;                          // Only valid for Remote add-ons
	AddOnFileList file_list;                // Get rid of this ASAP
	// TODO(Nordfriese): in the future, we might also want to include:
	// uploader username, upload date&time, average rating, number of votes, user comments, …
	// (but it would be pointless to implement that as long as we don't even have a real server)
};

// Sorted list of all add-ons mapped to whether they are currently enabled
using AddOnState = std::pair<AddOnInfo, bool>;
extern std::vector<AddOnState> g_addons;

extern const std::map<AddOnCategory, AddOnCategoryInfo> kAddOnCategories;
AddOnCategory get_category(const std::string&);

// Creates a string informing about missing or wrong-version add-ons
// for use in map- and savegame selection screens
std::string check_requirements(const AddOnRequirements&);

AddOnInfo preload_addon(const std::string&);

#endif  // end of include guard: WL_LOGIC_ADDONS_H
