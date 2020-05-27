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

#include "ui_fsmenu/addons.h"

#include <memory>

#include "base/i18n.h"
#include "io/profile.h"
#include "logic/filesystem_constants.h"
#include "ui_basic/messagebox.h"
#include "wlapplication_options.h"

constexpr int16_t kRowButtonSize = 32;
constexpr int16_t kRowButtonSpacing = 4;

// UI::Box by defaults limits its size to the window resolution. We use scrollbars,
// so we can and need to allow somewhat larger dimensions.
constexpr int32_t kHugeSize = std::numeric_limits<int32_t>::max() / 2;

AddOnsCtrl::AddOnsCtrl() : FullscreenMenuBase(),
		title_(this, 0, 0, get_w(), get_h() / 12, _("Add-Ons"), UI::Align::kCenter, g_gr->styles().font_style(UI::FontStyle::kFsMenuTitle)),
		warn_requirements_(this, 0, 0, get_w(), get_h() / 12, UI::PanelStyle::kFsMenu, "", UI::Align::kCenter),
		tabs_(this, UI::TabPanelStyle::kFsMenu),
		installed_addons_wrapper_(&tabs_, 0, 0, UI::Box::Vertical),
		browse_addons_wrapper_(&tabs_, 0, 0, UI::Box::Vertical),
		installed_addons_box_(&installed_addons_wrapper_, 0, 0, UI::Box::Vertical, kHugeSize, kHugeSize),
		browse_addons_box_(&browse_addons_wrapper_, 0, 0, UI::Box::Vertical, kHugeSize, kHugeSize),
		filter_settings_(&tabs_, 0, 0, UI::Box::Vertical),
		filter_name_box_(&filter_settings_, 0, 0, UI::Box::Horizontal),
		filter_buttons_box_(&filter_settings_, 0, 0, UI::Box::Horizontal),
		filter_name_(&filter_settings_, 0, 0, 100, UI::PanelStyle::kFsMenu),
		filter_category_(&filter_settings_, "filter_cat", 0, 0, 100, 8, kRowButtonSize, _("Filter by category"),
				UI::DropdownType::kTextual, UI::PanelStyle::kFsMenu, UI::ButtonStyle::kFsMenuSecondary),
		filter_verified_(&filter_settings_, Vector2i(0, 0), _("Verified only"), _("Show only verified add-ons in the Browse tab")),
		ok_(this, "ok", 0, 0, get_w() / 2, get_h() / 12, UI::ButtonStyle::kFsMenuPrimary, _("OK")),
		filter_apply_(&filter_buttons_box_, "f_apply", 0, 0, 24, 24, UI::ButtonStyle::kFsMenuPrimary, _("Apply"), _("Apply filters to lists")),
		filter_reset_(&filter_buttons_box_, "f_reset", 0, 0, 24, 24, UI::ButtonStyle::kFsMenuSecondary, _("Reset"), _("Reset the filters")),
		upgrade_all_(&filter_buttons_box_, "upgrade_all", 0, 0, 24, 24, UI::ButtonStyle::kFsMenuSecondary, _("Upgrade all"),
				_("Upgrade all installed add-ons for which a newer version is available")),
		refresh_(&filter_buttons_box_, "refresh", 0, 0, kRowButtonSize, kRowButtonSize, UI::ButtonStyle::kFsMenuSecondary,
				_("Refresh"), _("Refresh the list of add-ons available from the server")) {
	installed_addons_wrapper_.set_scrolling(true);
	browse_addons_wrapper_.set_scrolling(true);
	installed_addons_wrapper_.add(&installed_addons_box_, UI::Box::Resizing::kExpandBoth);
	browse_addons_wrapper_.add(&browse_addons_box_, UI::Box::Resizing::kExpandBoth);
	tabs_.add("my", _("Installed"), &installed_addons_wrapper_);
	tabs_.add("all", _("Browse"), &browse_addons_wrapper_);
	tabs_.add("filter", _("Filter"), &filter_settings_);

	filter_name_box_.add(new UI::Textarea(&filter_name_box_, _("Filter by text:"), UI::Align::kRight), UI::Box::Resizing::kFullSize);
	filter_name_box_.add_space(kRowButtonSpacing);
	filter_name_box_.add(&filter_name_, UI::Box::Resizing::kExpandBoth);

	filter_buttons_box_.add(&filter_apply_, UI::Box::Resizing::kExpandBoth);
	filter_buttons_box_.add_space(2 * kRowButtonSpacing);
	filter_buttons_box_.add(&filter_reset_, UI::Box::Resizing::kExpandBoth);
	filter_buttons_box_.add_space(2 * kRowButtonSpacing);
	filter_buttons_box_.add(&upgrade_all_, UI::Box::Resizing::kExpandBoth);
	filter_buttons_box_.add_space(2 * kRowButtonSpacing);
	filter_buttons_box_.add(&refresh_, UI::Box::Resizing::kExpandBoth);

	filter_settings_.add(&filter_name_box_, UI::Box::Resizing::kFullSize);
	filter_settings_.add_space(2 * kRowButtonSpacing);
	filter_settings_.add(&filter_category_, UI::Box::Resizing::kFullSize);
	filter_settings_.add_space(2 * kRowButtonSpacing);
	filter_settings_.add(&filter_verified_, UI::Box::Resizing::kFullSize);
	filter_settings_.add_space(2 * kRowButtonSpacing);
	filter_settings_.add(&filter_buttons_box_, UI::Box::Resizing::kFullSize);

	filter_verified_.set_state(true);
	filter_category_.add(_("Any"), "", nullptr, true);
	for (const auto& pair : kAddOnCategories) {
		if (pair.first != AddOnCategory::kNone) {
			filter_category_.add(pair.second.descname(), pair.second.internal_name);
		}
	}

	filter_apply_.set_enabled(false);
	filter_reset_.set_enabled(false);
	filter_name_.changed.connect([this]() {
		filter_apply_.set_enabled(true);
		filter_reset_.set_enabled(true);
	});
	filter_category_.selected.connect([this]() {
		filter_apply_.set_enabled(true);
		filter_reset_.set_enabled(true);
	});
	filter_verified_.clickedto.connect([this](bool) {
		filter_apply_.set_enabled(true);
		filter_reset_.set_enabled(true);
	});

	ok_.sigclicked.connect([this]() {
		clicked_ok();
	});
	refresh_.sigclicked.connect([this]() {
		refresh_remotes();
		tabs_.activate(1);
	});

	filter_name_.ok.connect([this]() {
		rebuild();
		filter_apply_.set_enabled(false);
		filter_reset_.set_enabled(true);
	});
	filter_apply_.sigclicked.connect([this]() {
		rebuild();
		filter_apply_.set_enabled(false);
		filter_reset_.set_enabled(true);
	});

	filter_reset_.sigclicked.connect([this]() {
		filter_name_.set_text("");
		filter_category_.select("");
		filter_verified_.set_state(true);
		rebuild();
		filter_apply_.set_enabled(false);
		filter_reset_.set_enabled(false);
	});
	upgrade_all_.sigclicked.connect([this]() {
		std::vector<std::pair<AddOnInfo, std::pair<bool /* full upgrade */, uint32_t /* remote i18n version */>>> upgrades;
		bool all_verified = true;
		size_t nr_full_updates = 0;
		for (const RemoteAddOnRow* r : browse_) {
			if (r->upgradeable()) {
				const bool full_upgrade = r->full_upgrade_possible();
				upgrades.push_back(std::make_pair(r->info(), std::make_pair(full_upgrade, r->info().i18n_version)));
				if (full_upgrade) {
					all_verified &= r->info().verified;
					++nr_full_updates;
				}
			}
		}
		assert(!upgrades.empty());
		if (nr_full_updates > 0 && (!all_verified || !(SDL_GetModState() & KMOD_CTRL))) {
			// We ask for confirmation only for real upgrades. i18n-only upgrades are done silently.
			std::string text = (boost::format(ngettext("Are you certain that you want to upgrade this %u add-on?\n",
					"Are you certain that you want to upgrade these %u add-ons?\n", nr_full_updates)) % nr_full_updates).str();
			for (const auto& pair : upgrades) {
				if (pair.second.first) {
					text += (boost::format(_("\n· %1$s (%2$s) by %3$s"))
							% pair.first.descname % (pair.first.verified ? _("verified") : _("NOT VERIFIED")) % pair.first.author).str();
				}
			}
			UI::WLMessageBox w(this, _("Upgrade All"), text, UI::WLMessageBox::MBoxType::kOkCancel);
			if (w.run<UI::Panel::Returncodes>() != UI::Panel::Returncodes::kOk) { return; }
		}
		for (const auto& pair : upgrades) {
			upgrade(pair.first.internal_name, pair.second.first, pair.second.second);
		}
		rebuild();
	});

	// prevent assert failures
	installed_addons_box_.set_size(100, 100);
	browse_addons_box_.set_size(100, 100);
	installed_addons_wrapper_.set_size(100, 100);
	browse_addons_wrapper_.set_size(100, 100);

	refresh_remotes();
}

AddOnsCtrl::~AddOnsCtrl() {
	std::string text;
	for (const auto& pair : g_addons) {
		if (!text.empty()) {
			text += ',';
		}
		text += pair.first.internal_name + ':' + (pair.second ? "true" : "false");
	}
	set_config_string("addons", text);
	write_config();
}

void AddOnsCtrl::refresh_remotes() {
	remotes_.clear();
	try {

		// TODO(Nordfriese): Connect to the add-on server when we have one
		// and fetch a list of all available add-ons
		throw wexception("Not yet implemented");

	} catch (const std::exception& e) {
		remotes_.push_back(AddOnInfo {
			"",
			_("Server Connection Error"),
			(boost::format(_("Unable to fetch the list of available add-ons from the server!<br>Error Message: %s"))
				% e.what()).str(),
			/** TRANSLATORS: This will be inserted into the string "Server Connection Error \n by %s" */
			_("a networking bug"),
			0, 0, AddOnCategory::kNone, {}, false
		});
	}
	rebuild();
}

bool AddOnsCtrl::matches_filter(const AddOnInfo& info, bool local) {
	if (info.internal_name.empty()) {
		// always show error messages
		return true;
	}
	if (!filter_category_.get_selected().empty() && filter_category_.get_selected() != kAddOnCategories.at(info.category).internal_name) {
		// wrong category
		return false;
	}
	if (!local && filter_verified_.get_state() && !info.verified) {
		// not verified
		return false;
	}

	if (filter_name_.text().empty()) {
		// no text filter given, so we accept it
		return true;
	}
	for (const std::string& text : {info.descname, info.author, info.internal_name, info.description}) {
		if (text.find(filter_name_.text()) != std::string::npos) {
			// text filter found
			return true;
		}
	}
	// doesn't match the text filter
	return false;
}

void AddOnsCtrl::rebuild() {
	const uint32_t scrollpos_i = installed_addons_wrapper_.get_scrollbar() ? installed_addons_wrapper_.get_scrollbar()->get_scrollpos() : 0;
	const uint32_t scrollpos_b = browse_addons_wrapper_.get_scrollbar() ? browse_addons_wrapper_.get_scrollbar()->get_scrollpos() : 0;
	installed_addons_box_.free_children();
	browse_addons_box_.free_children();
	installed_addons_box_.clear();
	browse_addons_box_.clear();
	installed_.clear();
	browse_.clear();
	assert(installed_addons_box_.get_nritems() == 0);
	assert(browse_addons_box_.get_nritems() == 0);

	size_t index = 0;
	std::vector<std::pair<AddOnInfo, bool>> addons_to_show;
	for (const auto& pair : g_addons) {
		if (matches_filter(pair.first, true)) {
			addons_to_show.push_back(pair);
		}
	}
	size_t nr_installed = addons_to_show.size();
	for (const auto& pair : addons_to_show) {
		if (index > 0) {
			installed_addons_box_.add_space(kRowButtonSize);
		}
		InstalledAddOnRow* i = new InstalledAddOnRow(&installed_addons_box_, this, pair.first, pair.second, index == 0, index + 1 == nr_installed);
		installed_addons_box_.add(i, UI::Box::Resizing::kFullSize);
		++index;
	}

	index = 0;
	std::vector<AddOnInfo> remotes_to_show;
	for (const AddOnInfo& a : remotes_) {
		if (matches_filter(a, false)) {
			remotes_to_show.push_back(a);
		}
	}
	nr_installed = remotes_to_show.size();
	bool has_upgrades = false;
	for (const AddOnInfo& a : remotes_to_show) {
		if (0 < index++) {
			browse_addons_box_.add_space(kRowButtonSize);
		}
		uint32_t installed = kNotInstalled;
		uint32_t installed_i18n = kNotInstalled;
		for (const auto& pair : g_addons) {
			if (pair.first.internal_name == a.internal_name) {
				installed = pair.first.version;
				installed_i18n = pair.first.i18n_version;
				break;
			}
		}
		RemoteAddOnRow* r = new RemoteAddOnRow(&browse_addons_box_, this, a, installed, installed_i18n);
		browse_addons_box_.add(r, UI::Box::Resizing::kFullSize);
		has_upgrades |= r->upgradeable();
	}

	if (installed_addons_wrapper_.get_scrollbar() && scrollpos_i) {
		installed_addons_wrapper_.get_scrollbar()->set_scrollpos(scrollpos_i);
	}
	if (browse_addons_wrapper_.get_scrollbar() && scrollpos_b) {
		browse_addons_wrapper_.get_scrollbar()->set_scrollpos(scrollpos_b);
	}

	upgrade_all_.set_enabled(has_upgrades);
	update_dependency_errors();
}

void AddOnsCtrl::update_dependency_errors() {
	std::vector<std::string> warn_requirements;
	for (auto addon = g_addons.begin(); addon != g_addons.end(); ++addon) {
		if (!addon->second && kAddOnCategories.at(addon->first.category).can_disable_addons) {
			// Disabled, so we don't care about dependencies
			continue;
		}
		// TODO(Nordfriese): Also warn if the add-on's requirements are present in the wrong order
		// (e.g. when A requires B,C but they are ordered C,B,A)
		for (const std::string& requirement : addon->first.requires) {
			std::vector<AddOnState>::iterator search_result = g_addons.end();
			bool too_late = false;
			for (auto search = g_addons.begin(); search != g_addons.end(); ++search) {
				if (search->first.internal_name == requirement) {
					search_result = search;
					break;
				}
				if (search == addon) {
					assert(!too_late);
					too_late = true;
				}
			}
			if (search_result == g_addons.end()) {
				warn_requirements.push_back((boost::format(_("· ‘%1$s’ requires ‘%2$s’ which could not be found"))
						% addon->first.descname % requirement).str());
			} else {
				if (!search_result->second && kAddOnCategories.at(search_result->first.category).can_disable_addons) {
					warn_requirements.push_back((boost::format(_("· ‘%1$s’ requires ‘%2$s’ which is disabled"))
							% addon->first.descname % search_result->first.descname).str());
				}
				if (too_late) {
					warn_requirements.push_back((boost::format(_("· ‘%1$s’ requires ‘%2$s’ which is listed below the requiring add-on"))
							% addon->first.descname % search_result->first.descname).str());
				}
			}
		}
	}
	if (warn_requirements.empty()) {
		warn_requirements_.set_text("");
		warn_requirements_.set_tooltip("");
	} else {
		const unsigned nr_warnings = warn_requirements.size();
		std::string list;
		for (const std::string& msg : warn_requirements) {
			if (!list.empty()) {
				list += "<br>";
			}
			list += msg;
		}
		warn_requirements_.set_text((boost::format("<rt><p>%s</p><p>%s</p></rt>")
				% g_gr->styles().font_style(UI::FontStyle::kFsMenuInfoPanelHeading).as_font_tag(
						(boost::format(ngettext(_("%u Dependency Error"), _("%u Dependency Errors"),
								nr_warnings)) % nr_warnings).str())
				% g_gr->styles().font_style(UI::FontStyle::kFsMenuInfoPanelParagraph).as_font_tag(list)).str());
		warn_requirements_.set_tooltip(_("Add-Ons with dependency errors may work incorrectly or prevent games and maps from loading."));
	}
	layout();
}

void AddOnsCtrl::layout() {
	FullscreenMenuBase::layout();

	title_.set_size(get_w(), get_h() / 16);
	title_.set_pos(Vector2i(0, get_h() / 16));

	ok_.set_size(get_w() / 2, get_h() / 20);
	ok_.set_pos(Vector2i(get_w() / 4, get_h() * 14 / 16));

	const bool has_warnings = !warn_requirements_.get_text().empty();
	warn_requirements_.set_size(get_w() * 2 / 3, has_warnings ? get_h() / 8 : 0);
	warn_requirements_.set_pos(Vector2i(get_w() / 6, get_h() * 12 / 16));
	tabs_.set_size(get_w() * 2 / 3, has_warnings ? get_h() * 9 / 16 : get_h() * 2 / 3);
	tabs_.set_pos(Vector2i(get_w() / 6, get_h() / 6));

	installed_addons_wrapper_.set_max_size(tabs_.get_w(), tabs_.get_h() - kRowButtonSize);
	browse_addons_wrapper_.set_max_size(tabs_.get_w(), tabs_.get_h() - kRowButtonSize);
}

// UNTESTED
static void install_translation(const std::string& temp_locale_path, const std::string& addon_name, bool is_upgrade) {
	assert(g_fs->file_exists(temp_locale_path));

	// NOTE:
	// gettext expects a directory structure such as
	// "~/.widelands/addons_i18n/nds/LC_MESSAGES/addon_name.wad.mo"
	// where "nds" is the language abbreviation and "addon_name.wad" the add-on's name.
	// If we use a different structure, gettext will not find the translations!

	const std::string temp_filename = g_fs->fs_filename(temp_locale_path.c_str());  // nds.mo
	const std::string locale = temp_filename.substr(0, temp_filename.find('.'));    // nds

	const std::string new_locale_dir =
	   kAddOnLocaleDir + g_fs->file_separator() + locale +
	   g_fs->file_separator() + "LC_MESSAGES";  // addons_i18n/nds/LC_MESSAGES
	g_fs->ensure_directory_exists(new_locale_dir);

	const std::string new_locale_path = new_locale_dir + g_fs->file_separator() + addon_name + ".mo";

	assert(!g_fs->is_directory(new_locale_path));
	if (is_upgrade && g_fs->file_exists(new_locale_path)) {
		// delete the outdated translation if present
		g_fs->fs_unlink(new_locale_path);
	}
	assert(!g_fs->file_exists(new_locale_path));

	// move translation file from temp location to the correct place
	g_fs->fs_rename(temp_locale_path, new_locale_path);

	assert(g_fs->file_exists(new_locale_path));
	assert(!g_fs->file_exists(temp_locale_path));
}

// UNTESTED
// TODO(Nordfriese): install() and upgrade() should also (recursively) install the add-on's requirements
void AddOnsCtrl::install(const std::string& name, const uint32_t i18n_version) {
	g_fs->ensure_directory_exists(kAddOnDir);
	const std::string path = download_addon(name);
	if (path.empty()) {
		// downloading failed
		return;
	}

	// Install the add-on
	{
		const std::string new_path = kAddOnDir + g_fs->file_separator() + name;

		assert(g_fs->file_exists(path));
		assert(!g_fs->file_exists(new_path));
		assert(!g_fs->is_directory(new_path));

		g_fs->fs_rename(path, new_path);

		assert(!g_fs->file_exists(path));
		assert(g_fs->file_exists(new_path));
	}

	// Now download the translations
	for (const std::string& temp_locale_path : download_i18n(name, i18n_version)) {
		install_translation(temp_locale_path, name, false);
	}

	g_addons.push_back(std::make_pair(preload_addon(name), true));
}

// UNTESTED
// Upgrades the specified add-on. If `full_upgrade` is `false`, only translations will be updated.
void AddOnsCtrl::upgrade(const std::string& name, bool full_upgrade, const uint32_t i18n_version) {
	if (full_upgrade) {
		g_fs->ensure_directory_exists(kAddOnDir);
		const std::string path = download_addon(name);
		if (path.empty()) {
			// downloading failed
			return;
		}

		// Upgrade the add-on
		const std::string new_path = kAddOnDir + g_fs->file_separator() + name;

		assert(g_fs->file_exists(path));
		assert(g_fs->file_exists(new_path) ^ g_fs->is_directory(new_path));

		g_fs->fs_unlink(new_path);  // Uninstall the old version…

		assert(!g_fs->file_exists(new_path));
		assert(!g_fs->is_directory(new_path));

		g_fs->fs_rename(path, new_path);  // …and replace with the new one.

		assert(g_fs->file_exists(new_path));
		assert(!g_fs->file_exists(path));
	}

	// Now download the translations
	for (const std::string& temp_locale_path : download_i18n(name, i18n_version)) {
		install_translation(temp_locale_path, name, true);
	}

	for (auto& pair : g_addons) {
		if (pair.first.internal_name == name) {
			pair.first = preload_addon(name);
			return;
		}
	}
	NEVER_HERE();
}

// UNTESTED
std::string AddOnsCtrl::download_addon(const std::string& name) {
	try {

		// TODO(Nordfriese): Request the ZIP-file with the given name (e.g. "cool_feature.wad") from
		// the server, download it into a temporary location (e.g. ~/.widelands/temp/cool_feature.wad),
		// and return the path to the downloaded file.
		throw wexception("Not yet implemented");

	} catch (const std::exception& e) {
		UI::WLMessageBox w(this, _("Error"), (boost::format(
				_("The add-on '%1$s' could not be downloaded from the server. Installing/upgrading this add-on will be skipped.\n\nError Message:\n%2$s"))
				% name.c_str() % e.what()).str(), UI::WLMessageBox::MBoxType::kOk);
		w.run<UI::Panel::Returncodes>();
	}
	return "";
}

// UNTESTED
// NOTE: I expect elsewhere that every file's filename will be called
// "nds.mo" (where nds is the language's abbreviation).
// I don't care about the path of the temp file.
std::set<std::string> AddOnsCtrl::download_i18n(const std::string& name, const uint32_t i18n_version) {
	try {

		// TODO(Nordfriese): Request the MO files for the given add-on in all languages from the server,
		// download them into temporary locations (e.g. ~/.widelands/temp/nds.mo),
		// and return the paths to the downloaded files.
		throw wexception("Not yet implemented");





		// If the translations were downloaded correctly, we also update the i18n version info
		Profile prof(kAddOnLocaleVersions.c_str());
		prof.pull_section("global").set_natural(name.c_str(), i18n_version);
		prof.write(kAddOnLocaleVersions.c_str(), false);

	} catch (const std::exception& e) {
		UI::WLMessageBox w(this, _("Error"), (boost::format(
				_("The translation files for the add-on '%1$s' could not be downloaded from the server. "
				"Installing/upgrading the translations for this add-on will be skipped.\n\nError Message:\n%2$s"))
				% name.c_str() % e.what()).str(), UI::WLMessageBox::MBoxType::kOk);
		w.run<UI::Panel::Returncodes>();
	}
	return {};
}

static void uninstall(AddOnsCtrl* ctrl, const AddOnInfo& info) {
	if (!(SDL_GetModState() & KMOD_CTRL)) {
		UI::WLMessageBox w(ctrl, _("Uninstall"), (boost::format(_("Are you certain that you want to uninstall this add-on?\n\n"
			"%1$s\n"
			"by %2$s\n"
			"Version %3$u\n"
			"Category: %4$s\n"
			"%5$s\n"
			))
			% info.descname
			% info.author
			% info.version
			% kAddOnCategories.at(info.category).descname()
			% info.description
			).str(), UI::WLMessageBox::MBoxType::kOkCancel);
		if (w.run<UI::Panel::Returncodes>() != UI::Panel::Returncodes::kOk) { return; }
	}

	// Delete the add-on and its translations
	g_fs->fs_unlink(kAddOnDir + g_fs->file_separator() + info.internal_name);
	g_fs->fs_unlink(kAddOnLocaleDir + g_fs->file_separator() + info.internal_name);

	for (auto it = g_addons.begin(); it != g_addons.end(); ++it) {
		if (it->first.internal_name == info.internal_name) {
			g_addons.erase(it);
			return ctrl->rebuild();
		}
	}
	NEVER_HERE();
}

InstalledAddOnRow::InstalledAddOnRow(Panel* parent, AddOnsCtrl* ctrl, const AddOnInfo& info, bool enabled, bool is_first, bool is_last)
	: UI::Panel(parent, 0, 0, 3 * kRowButtonSize, 3 * kRowButtonSize),
	move_up_(this, "up", 0, 0, 24, 24, UI::ButtonStyle::kFsMenuSecondary, g_gr->images().get("images/ui_basic/scrollbar_up.png"), _("Move up")),
	move_down_(this, "down", 0, 0, 24, 24, UI::ButtonStyle::kFsMenuSecondary, g_gr->images().get("images/ui_basic/scrollbar_down.png"), _("Move down")),
	uninstall_(this, "uninstall", 0, 0, 24, 24, UI::ButtonStyle::kFsMenuSecondary, g_gr->images().get("images/wui/menus/exit.png"), _("Uninstall")),
	toggle_enabled_(kAddOnCategories.at(info.category).can_disable_addons ? new UI::Button(this, "on-off", 0, 0, 24, 24,
			UI::ButtonStyle::kFsMenuSecondary, g_gr->images().get(
					enabled ? "images/ui_basic/checkbox_checked.png" : "images/ui_basic/checkbox_empty.png"),
					enabled ? _("Disable") : _("Enable"), UI::Button::VisualState::kFlat) : nullptr),
	category_(this, g_gr->images().get(kAddOnCategories.at(info.category).icon)),
	version_(this, 0, 0, 0, 0, std::to_string(static_cast<int>(info.version)), UI::Align::kCenter, g_gr->styles().font_style(UI::FontStyle::kFsMenuTitle)),
	txt_(this, 0, 0, 24, 24, UI::PanelStyle::kFsMenu, (boost::format("<rt>%s<p>%s</p><p>%s</p></rt>")
		% g_gr->styles().font_style(UI::FontStyle::kFsMenuInfoPanelHeading).as_font_tag(info.descname)
		% g_gr->styles().font_style(UI::FontStyle::kChatWhisper).as_font_tag((boost::format(_("by %s")) % info.author).str())
		% g_gr->styles().font_style(UI::FontStyle::kFsMenuInfoPanelParagraph).as_font_tag(info.description)).str()) {

	uninstall_.sigclicked.connect([this, ctrl, info]() {
		uninstall(ctrl, info);
	});
	move_up_.sigclicked.connect([this, ctrl, info]() {
		auto it = g_addons.begin();
		for (; it->first.internal_name != info.internal_name; ++it);
		const bool state = it->second;
		it = g_addons.erase(it);
		--it;
		g_addons.insert(it, std::make_pair(info, state));
		ctrl->rebuild();
	});
	move_down_.sigclicked.connect([this, ctrl, info]() {
		auto it = g_addons.begin();
		for (; it->first.internal_name != info.internal_name; ++it);
		const bool state = it->second;
		it = g_addons.erase(it);
		++it;
		g_addons.insert(it, std::make_pair(info, state));
		ctrl->rebuild();
	});
	if (toggle_enabled_) {
		toggle_enabled_->sigclicked.connect([this, ctrl, info]() {
			for (auto& pair : g_addons) {
				if (pair.first.internal_name == info.internal_name) {
					pair.second = !pair.second;
					toggle_enabled_->set_pic(g_gr->images().get(
							pair.second ? "images/ui_basic/checkbox_checked.png" : "images/ui_basic/checkbox_empty.png"));
					toggle_enabled_->set_tooltip(pair.second ? _("Disable") : _("Enable"));
					return ctrl->update_dependency_errors();
				}
			}
			NEVER_HERE();
		});
	}
	move_up_.set_enabled(!is_first);
	move_down_.set_enabled(!is_last);
	category_.set_handle_mouse(true);
	category_.set_tooltip((boost::format(_("Category: %s")) % kAddOnCategories.at(info.category).descname()).str());
	version_.set_handle_mouse(true);
	version_.set_tooltip(_("Version"));
	layout();
}

void InstalledAddOnRow::layout() {
	UI::Panel::layout();
	if (get_w() <= 2 * kRowButtonSize + 2 * kRowButtonSpacing) {
		// size not yet set
		return;
	}
	set_desired_size(get_w(), 3 * kRowButtonSize);
	for (UI::Panel* p : std::vector<UI::Panel*>{&move_up_, &move_down_, &uninstall_, &category_, &version_}) {
		p->set_size(kRowButtonSize, kRowButtonSize);
	}
	if (toggle_enabled_) {
		toggle_enabled_->set_size(kRowButtonSize, kRowButtonSize);
		toggle_enabled_->set_pos(Vector2i(get_w() - kRowButtonSize, kRowButtonSize));
	}
	move_up_.set_pos(Vector2i(get_w() - kRowButtonSize, 0));
	move_down_.set_pos(Vector2i(get_w() - kRowButtonSize, 2 * kRowButtonSize));
	category_.set_pos(Vector2i(get_w() - 2 * kRowButtonSize - kRowButtonSpacing, 0));
	version_.set_pos(Vector2i(get_w() - 2 * kRowButtonSize - kRowButtonSpacing, kRowButtonSize));
	uninstall_.set_pos(Vector2i(get_w() - 2 * kRowButtonSize - kRowButtonSpacing, 2 * kRowButtonSize));
	txt_.set_size(get_w() - 2 * kRowButtonSize - 2 * kRowButtonSpacing, 3 * kRowButtonSize);
	txt_.set_pos(Vector2i(0, 0));
}

RemoteAddOnRow::RemoteAddOnRow(Panel* parent, AddOnsCtrl* ctrl, const AddOnInfo& info, uint32_t installed_version, uint32_t installed_i18n_version)
	: UI::Panel(parent, 0, 0, 3 * kRowButtonSize, 3 * kRowButtonSize),
	info_(info),
	install_(this, "install", 0, 0, 24, 24, UI::ButtonStyle::kFsMenuSecondary, g_gr->images().get("images/ui_basic/continue.png"), _("Install")),
	upgrade_(this, "upgrade", 0, 0, 24, 24, UI::ButtonStyle::kFsMenuSecondary, g_gr->images().get("images/wui/buildings/menu_up_train.png"), _("Upgrade")),
	uninstall_(this, "uninstall", 0, 0, 24, 24, UI::ButtonStyle::kFsMenuSecondary, g_gr->images().get("images/wui/menus/exit.png"), _("Uninstall")),
	category_(this, g_gr->images().get(kAddOnCategories.at(info.category).icon)),
	verified_(this, g_gr->images().get(info.verified ? "images/ui_basic/list_selected.png" : "images/ui_basic/stop.png")),
	version_(this, 0, 0, 0, 0, std::to_string(static_cast<int>(info.version)), UI::Align::kCenter, g_gr->styles().font_style(UI::FontStyle::kFsMenuTitle)),
	txt_(this, 0, 0, 24, 24, UI::PanelStyle::kFsMenu, (boost::format("<rt>%s<p>%s</p><p>%s</p></rt>")
		% g_gr->styles().font_style(UI::FontStyle::kFsMenuInfoPanelHeading).as_font_tag(info.descname)
		% g_gr->styles().font_style(UI::FontStyle::kChatWhisper).as_font_tag((boost::format(_("by %s")) % info.author).str())
		% g_gr->styles().font_style(UI::FontStyle::kFsMenuInfoPanelParagraph).as_font_tag(info.description)).str()),
	full_upgrade_possible_(installed_version < info.version) {

	assert(installed_version <= info.version);
	assert(installed_i18n_version <= info.i18n_version);

	uninstall_.sigclicked.connect([this, ctrl, info]() {
		uninstall(ctrl, info);
	});
	install_.sigclicked.connect([this, ctrl, info]() {
		// Ctrl-click skips the confirmation. Never skip for non-verified stuff though.
		if (!info.verified || !(SDL_GetModState() & KMOD_CTRL)) {
			UI::WLMessageBox w(ctrl, _("Install"), (boost::format(_("Are you certain that you want to install this add-on?\n\n"
				"%1$s\n"
				"by %2$s\n"
				"%3$s\n"
				"Version %4$u\n"
				"Category: %5$s\n"
				"%6$s\n"
				))
				% info.descname
				% info.author
				% (info.verified ? _("Verified") : _("NOT VERIFIED"))
				% info.version
				% kAddOnCategories.at(info.category).descname()
				% info.description
				).str(), UI::WLMessageBox::MBoxType::kOkCancel);
			if (w.run<UI::Panel::Returncodes>() != UI::Panel::Returncodes::kOk) { return; }
		}
		ctrl->install(info.internal_name, info.i18n_version);
		ctrl->rebuild();
	});
	upgrade_.sigclicked.connect([this, ctrl, info, installed_version]() {
		if (!info.verified || !(SDL_GetModState() & KMOD_CTRL)) {
			UI::WLMessageBox w(ctrl, _("Upgrade"), (boost::format(_("Are you certain that you want to upgrade this add-on?\n\n"
				"%1$s\n"
				"by %2$s\n"
				"%3$s\n"
				"Installed version: %4$u\n"
				"Available version: %5$u\n"
				"Category: %6$s\n"
				"%7$s\n"
				))
				% info.descname
				% info.author
				% (info.verified ? _("Verified") : _("NOT VERIFIED"))
				% installed_version
				% info.version
				% kAddOnCategories.at(info.category).descname()
				% info.description
				).str(), UI::WLMessageBox::MBoxType::kOkCancel);
			if (w.run<UI::Panel::Returncodes>() != UI::Panel::Returncodes::kOk) { return; }
		}
		ctrl->upgrade(info.internal_name, full_upgrade_possible_, info.i18n_version);
		ctrl->rebuild();
	});
	if (info.internal_name.empty()) {
		install_.set_enabled(false);
		upgrade_.set_enabled(false);
		uninstall_.set_enabled(false);
	} else if (installed_version == kNotInstalled) {
		uninstall_.set_enabled(false);
		upgrade_.set_enabled(false);
	} else {
		install_.set_enabled(false);
		upgrade_.set_enabled(full_upgrade_possible_ || installed_i18n_version < info.i18n_version);
	}
	category_.set_handle_mouse(true);
	category_.set_tooltip((boost::format(_("Category: %s")) % kAddOnCategories.at(info.category).descname()).str());
	version_.set_handle_mouse(true);
	version_.set_tooltip(_("Version"));
	verified_.set_handle_mouse(true);
	verified_.set_tooltip(info.internal_name.empty() ? _("Error") : info.verified ? _("Verified by the Widelands Development Team") :
		_("This add-on was not checked by the Widelands Development Team yet. We cannot guarantee that it does not contain harmful or offensive content."));
	layout();
}

void RemoteAddOnRow::layout() {
	UI::Panel::layout();
	if (get_w() <= 2 * kRowButtonSize + 2 * kRowButtonSpacing) {
		// size not yet set
		return;
	}
	set_desired_size(get_w(), 3 * kRowButtonSize);
	for (UI::Panel* p : std::vector<UI::Panel*>{&install_, &uninstall_, &upgrade_, &category_, &version_, &verified_}) {
		p->set_size(kRowButtonSize, kRowButtonSize);
	}
	install_.set_pos(Vector2i(get_w() - kRowButtonSize, 0));
	upgrade_.set_pos(Vector2i(get_w() - kRowButtonSize, kRowButtonSize));
	uninstall_.set_pos(Vector2i(get_w() - kRowButtonSize, 2 * kRowButtonSize));
	category_.set_pos(Vector2i(get_w() - kRowButtonSize * 2 - kRowButtonSpacing, 0));
	version_.set_pos(Vector2i(get_w() - kRowButtonSize * 2 - kRowButtonSpacing, kRowButtonSize));
	verified_.set_pos(Vector2i(get_w() - kRowButtonSize * 2 - kRowButtonSpacing, 2 * kRowButtonSize));
	txt_.set_size(get_w() - 2 * kRowButtonSize - 2 * kRowButtonSpacing, 3 * kRowButtonSize);
	txt_.set_pos(Vector2i(0, 0));
}

bool RemoteAddOnRow::upgradeable() const {
	return upgrade_.enabled();
}
