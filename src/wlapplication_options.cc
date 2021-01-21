/*
 * Copyright (C) 2012-2020 by the Widelands Development Team
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

#include "wlapplication_options.h"

#include <cassert>
#include <cstdlib>
#include <map>
#include <memory>
#include <set>
#include <vector>

#include <boost/format.hpp>

#include "base/i18n.h"
#include "base/log.h"
#include "graphic/text_layout.h"
#include "io/filesystem/disk_filesystem.h"
#include "logic/filesystem_constants.h"

static Profile g_options(Profile::err_log);

static std::unique_ptr<FileSystem> config_dir = nullptr;

void check_config_used() {
	g_options.check_used();
}

Section& get_config_section() {
	return g_options.pull_section("global");
}

Section& get_config_section(const std::string& section) {
	return g_options.pull_section(section.c_str());
}

Section* get_config_section_ptr(const std::string& section) {
	return g_options.get_section(section);
}

bool get_config_bool(const std::string& name, const bool dflt) {
	return g_options.pull_section("global").get_bool(name.c_str(), dflt);
}

bool get_config_bool(const std::string& section, const std::string& name, const bool dflt) {
	return g_options.pull_section(section.c_str()).get_bool(name.c_str(), dflt);
}

int32_t get_config_int(const std::string& name, const int32_t dflt) {
	return g_options.pull_section("global").get_int(name.c_str(), dflt);
}

int32_t get_config_int(const std::string& section, const std::string& name, const int32_t dflt) {
	return g_options.pull_section(section.c_str()).get_int(name.c_str(), dflt);
}

uint32_t get_config_natural(const std::string& name, const uint32_t dflt) {
	return g_options.pull_section("global").get_natural(name.c_str(), dflt);
}

uint32_t get_config_natural(const std::string& section, const std::string& name, uint32_t dflt) {
	return g_options.pull_section(section.c_str()).get_natural(name.c_str(), dflt);
}

std::string get_config_string(const std::string& name, const std::string& dflt) {
	return g_options.pull_section("global").get_string(name.c_str(), dflt.c_str());
}

std::string
get_config_string(const std::string& section, const std::string& name, const std::string& dflt) {
	return g_options.pull_section(section.c_str()).get_string(name.c_str(), dflt.c_str());
}

Section& get_config_safe_section() {
	return g_options.get_safe_section("global");
}

Section& get_config_safe_section(const std::string& section) {
	return g_options.get_safe_section(section);
}

void set_config_bool(const std::string& name, const bool value) {
	g_options.pull_section("global").set_bool(name.c_str(), value);
}

void set_config_bool(const std::string& section, const std::string& name, const bool value) {
	g_options.pull_section(section.c_str()).set_bool(name.c_str(), value);
}

void set_config_int(const std::string& name, int32_t value) {
	g_options.pull_section("global").set_int(name.c_str(), value);
}

void set_config_int(const std::string& section, const std::string& name, const int32_t value) {
	g_options.pull_section(section.c_str()).set_int(name.c_str(), value);
}

void set_config_string(const std::string& name, const std::string& value) {
	g_options.pull_section("global").set_string(name.c_str(), value.c_str());
}

void set_config_string(const std::string& section,
                       const std::string& name,
                       const std::string& value) {
	g_options.pull_section(section.c_str()).set_string(name.c_str(), value.c_str());
}

struct KeyboardShortcutInfo {
	enum class Scope {
		kGlobal,  // special value that intersects with all other scopes

		kMainMenu,
		kEditor,
		kGame,
	};

	const std::set<Scope> scopes;
	const SDL_Keysym default_shortcut;
	SDL_Keysym current_shortcut;
	const std::string internal_name;
	const std::function<std::string()> descname;

	KeyboardShortcutInfo(const std::set<Scope>& s,
	                     const SDL_Keysym& sym,
	                     const std::string& n,
	                     const std::function<std::string()>& f)
	   : scopes(s), default_shortcut(sym), current_shortcut(sym), internal_name(n), descname(f) {
	}
};

static inline SDL_Keysym keysym(const SDL_Keycode c, unsigned short mod = 0) {
	return SDL_Keysym{SDL_GetScancodeFromKey(c), c, mod, 0};
}

static std::map<KeyboardShortcut, KeyboardShortcutInfo> shortcuts_ = {
   {KeyboardShortcut::kMainMenuNew, KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kMainMenu},
                                                         keysym(SDLK_n),
                                                         "mainmenu_new",
                                                         []() { return _("New Game"); })},
   {KeyboardShortcut::kMainMenuLoad, KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kMainMenu},
                                                          keysym(SDLK_l),
                                                          "mainmenu_load",
                                                          []() { return _("Load Game"); })},
   {KeyboardShortcut::kMainMenuReplay, KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kMainMenu},
                                                            keysym(SDLK_r),
                                                            "mainmenu_replay",
                                                            []() { return _("Watch Replay"); })},
   {KeyboardShortcut::kMainMenuRandomMatch,
    KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kMainMenu},
                         keysym(SDLK_z),
                         "mainmenu_random",
                         []() { return _("New Random Game"); })},
   {KeyboardShortcut::kMainMenuTutorial,
    KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kMainMenu},
                         keysym(SDLK_t),
                         "mainmenu_tutorial",
                         []() { return _("Tutorials"); })},
   {KeyboardShortcut::kMainMenuCampaign,
    KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kMainMenu},
                         keysym(SDLK_c),
                         "mainmenu_campaign",
                         []() { return _("Campaigns"); })},
   {KeyboardShortcut::kMainMenuSP, KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kMainMenu},
                                                        keysym(SDLK_s),
                                                        "mainmenu_sp",
                                                        []() { return _("Singleplayer"); })},
   {KeyboardShortcut::kMainMenuMP, KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kMainMenu},
                                                        keysym(SDLK_m),
                                                        "mainmenu_mp",
                                                        []() { return _("Multiplayer"); })},
   {KeyboardShortcut::kMainMenuE, KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kMainMenu},
                                                       keysym(SDLK_e),
                                                       "mainmenu_e",
                                                       []() { return _("Editor"); })},
   {KeyboardShortcut::kMainMenuEditorLoad,
    KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kMainMenu},
                         keysym(SDLK_b),
                         "mainmenu_editor_load",
                         []() { return _("Editor – Load Map"); })},
   {KeyboardShortcut::kMainMenuEditorNew,
    KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kMainMenu},
                         keysym(SDLK_k),
                         "mainmenu_editor_new",
                         []() { return _("Editor – New Map"); })},
   {KeyboardShortcut::kMainMenuEditorRandom,
    KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kMainMenu},
                         keysym(SDLK_y),
                         "mainmenu_editor_random",
                         []() { return _("Editor – New Random Map"); })},
   {KeyboardShortcut::kMainMenuContinueEditing,
    KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kMainMenu},
                         keysym(SDLK_w),
                         "mainmenu_editor_continue",
                         []() { return _("Continue Editing"); })},
   {KeyboardShortcut::kMainMenuContinuePlaying,
    KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kMainMenu},
                         keysym(SDLK_c),
                         "mainmenu_continue",
                         []() { return _("Continue Playing"); })},
   {KeyboardShortcut::kMainMenuQuit, KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kMainMenu},
                                                          keysym(SDLK_ESCAPE),
                                                          "mainmenu_quit",
                                                          []() { return _("Exit Widelands"); })},
   {KeyboardShortcut::kMainMenuAbout, KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kMainMenu},
                                                           keysym(SDLK_F1),
                                                           "mainmenu_about",
                                                           []() { return _("About"); })},
   {KeyboardShortcut::kMainMenuAddons, KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kMainMenu},
                                                            keysym(SDLK_a),
                                                            "mainmenu_addons",
                                                            []() { return _("Add-Ons"); })},
   {KeyboardShortcut::kMainMenuLAN, KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kMainMenu},
                                                         keysym(SDLK_p),
                                                         "mainmenu_lan",
                                                         []() { return _("LAN / Direct IP"); })},
   {KeyboardShortcut::kMainMenuLobby, KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kMainMenu},
                                                           keysym(SDLK_j),
                                                           "mainmenu_lobby",
                                                           []() { return _("Metaserver Lobby"); })},
   {KeyboardShortcut::kMainMenuLogin, KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kMainMenu},
                                                           keysym(SDLK_u),
                                                           "mainmenu_login",
                                                           []() { return _("Internet Login"); })},
   {KeyboardShortcut::kMainMenuOptions, KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kMainMenu},
                                                             keysym(SDLK_o),
                                                             "mainmenu_options",
                                                             []() { return _("Options"); })},

   {KeyboardShortcut::kCommonBuildhelp,
    KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kGame, KeyboardShortcutInfo::Scope::kEditor},
                         keysym(SDLK_SPACE),
                         "buildhelp",
                         []() { return _("Toggle Buildhelp"); })},
   {KeyboardShortcut::kCommonMinimap,
    KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kGame, KeyboardShortcutInfo::Scope::kEditor},
                         keysym(SDLK_m),
                         "minimap",
                         []() { return _("Toggle Minimap"); })},
   {KeyboardShortcut::kCommonEncyclopedia,
    KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kGame, KeyboardShortcutInfo::Scope::kEditor},
                         keysym(SDLK_F1),
                         "encyclopedia",
                         []() { return _("Encyclopedia"); })},
   {KeyboardShortcut::kCommonFullscreen,
    KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kGlobal},
                         keysym(SDLK_f, KMOD_CTRL),
                         "fullscreen",
                         []() { return _("Toggle Fullscreen"); })},
   {KeyboardShortcut::kCommonScreenshot,
    KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kGlobal},
                         keysym(SDLK_F11),
                         "screenshot",
                         []() { return _("Take Screenshot"); })},
   {KeyboardShortcut::kCommonZoomIn,
    KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kGame, KeyboardShortcutInfo::Scope::kEditor},
                         keysym(SDLK_PLUS, KMOD_CTRL),
                         "zoom_in",
                         []() { return _("Zoom In"); })},
   {KeyboardShortcut::kCommonZoomOut,
    KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kGame, KeyboardShortcutInfo::Scope::kEditor},
                         keysym(SDLK_MINUS, KMOD_CTRL),
                         "zoom_out",
                         []() { return _("Zoom Out"); })},
   {KeyboardShortcut::kCommonZoomReset,
    KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kGame, KeyboardShortcutInfo::Scope::kEditor},
                         keysym(SDLK_0, KMOD_CTRL),
                         "zoom_reset",
                         []() { return _("Reset Zoom"); })},

   {KeyboardShortcut::kEditorMenu,
    KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kEditor},
                         keysym(SDLK_h),
                         "editor_menu",
                         []() { return _("Menu"); })},
   {KeyboardShortcut::kEditorSave,
    KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kEditor},
                         keysym(SDLK_s, KMOD_CTRL),
                         "editor_save",
                         []() { return _("Save Map"); })},
   {KeyboardShortcut::kEditorLoad,
    KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kEditor},
                         keysym(SDLK_l, KMOD_CTRL),
                         "editor_load",
                         []() { return _("Load Map"); })},
   {KeyboardShortcut::kEditorUndo,
    KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kEditor},
                         keysym(SDLK_z, KMOD_CTRL),
                         "editor_undo",
                         []() { return _("Undo"); })},
   {KeyboardShortcut::kEditorRedo,
    KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kEditor},
                         keysym(SDLK_y, KMOD_CTRL),
                         "editor_redo",
                         []() { return _("Redo"); })},
   {KeyboardShortcut::kEditorTools,
    KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kEditor},
                         keysym(SDLK_t),
                         "editor_tools",
                         []() { return _("Tools"); })},
   {KeyboardShortcut::kEditorInfo,
    KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kEditor},
                         keysym(SDLK_i),
                         "editor_info",
                         []() { return _("Info Tool"); })},
   {KeyboardShortcut::kEditorPlayers,
    KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kEditor},
                         keysym(SDLK_p),
                         "editor_players",
                         []() { return _("Players Menu"); })},
   {KeyboardShortcut::kEditorShowhideGrid,
    KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kEditor},
                         keysym(SDLK_g),
                         "editor_showhide_",
                         []() { return _("Toggle Grid"); })},
   {KeyboardShortcut::kEditorShowhideImmovables,
    KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kEditor},
                         keysym(SDLK_2, KMOD_CTRL),
                         "editor_showhide_immovables",
                         []() { return _("Toggle Immovables"); })},
   {KeyboardShortcut::kEditorShowhideCritters,
    KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kEditor},
                         keysym(SDLK_3, KMOD_CTRL),
                         "editor_showhide_critters",
                         []() { return _("Toggle Critters"); })},
   {KeyboardShortcut::kEditorShowhideResources,
    KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kEditor},
                         keysym(SDLK_4, KMOD_CTRL),
                         "editor_showhide_resources",
                         []() { return _("Toggle Resources"); })},

   {KeyboardShortcut::kInGameShowhideCensus,
    KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kGame},
                         keysym(SDLK_c),
                         "game_showhide_census",
                         []() { return _("Toggle Census"); })},
   {KeyboardShortcut::kInGameShowhideStats,
    KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kGame},
                         keysym(SDLK_s),
                         "game_showhide_stats",
                         []() { return _("Toggle Statistics Labels"); })},
   {KeyboardShortcut::kInGameShowhideSoldiers,
    KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kGame},
                         keysym(SDLK_l),
                         "game_showhide_soldiers",
                         []() { return _("Toggle Soldier Levels"); })},
   {KeyboardShortcut::kInGameShowhideBuildings,
    KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kGame},
                         keysym(SDLK_u),
                         "game_showhide_buildings",
                         []() { return _("Toggle Buildings Visibility"); })},
   {KeyboardShortcut::kInGameShowhideWorkareas,
    KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kGame},
                         keysym(SDLK_w),
                         "game_showhide_workareas",
                         []() { return _("Toggle Overlapping Workareas"); })},
   {KeyboardShortcut::kInGameStatsGeneral,
    KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kGame},
                         keysym(SDLK_g),
                         "game_stats_general",
                         []() { return _("General Statistics"); })},
   {KeyboardShortcut::kInGameStatsWares,
    KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kGame},
                         keysym(SDLK_p),
                         "game_stats_wares",
                         []() { return _("Ware Statistics"); })},
   {KeyboardShortcut::kInGameStatsBuildings,
    KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kGame},
                         keysym(SDLK_b),
                         "game_stats_buildings",
                         []() { return _("Building Statistics"); })},
   {KeyboardShortcut::kInGameStatsStock,
    KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kGame},
                         keysym(SDLK_i),
                         "game_stats_stock",
                         []() { return _("Stock Inventory"); })},
   {KeyboardShortcut::kInGameStatsSoldiers,
    KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kGame},
                         keysym(SDLK_x),
                         "game_stats_soldiers",
                         []() { return _("Soldier Statistics"); })},
   {KeyboardShortcut::kInGameStatsSeafaring,
    KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kGame},
                         keysym(SDLK_e),
                         "game_stats_seafaring",
                         []() { return _("Seafaring Statistics"); })},
   {KeyboardShortcut::kInGameObjectives,
    KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kGame},
                         keysym(SDLK_o),
                         "game_objectives",
                         []() { return _("Objectives"); })},
   {KeyboardShortcut::kInGameMessages,
    KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kGame},
                         keysym(SDLK_n),
                         "game_messages",
                         []() { return _("Messages"); })},
   {KeyboardShortcut::kInGameSpeedDown,
    KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kGame},
                         keysym(SDLK_PAGEDOWN),
                         "game_speed_down",
                         []() { return _("Decrease Game Speed by 1×"); })},
   {KeyboardShortcut::kInGameSpeedDownSlow,
    KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kGame},
                         keysym(SDLK_PAGEDOWN, KMOD_SHIFT),
                         "game_speed_down_slow",
                         []() { return _("Decrease Game Speed by 0.25×"); })},
   {KeyboardShortcut::kInGameSpeedDownFast,
    KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kGame},
                         keysym(SDLK_PAGEDOWN, KMOD_CTRL),
                         "game_speed_down_fast",
                         []() { return _("Decrease Game Speed by 10×"); })},
   {KeyboardShortcut::kInGameSpeedUp,
    KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kGame},
                         keysym(SDLK_PAGEUP),
                         "game_speed_up",
                         []() { return _("Increase Game Speed by 1×"); })},
   {KeyboardShortcut::kInGameSpeedUpSlow,
    KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kGame},
                         keysym(SDLK_PAGEUP, KMOD_SHIFT),
                         "game_speed_up_slow",
                         []() { return _("Increase Game Speed by 0.25×"); })},
   {KeyboardShortcut::kInGameSpeedUpFast,
    KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kGame},
                         keysym(SDLK_PAGEUP, KMOD_CTRL),
                         "game_speed_up_fast",
                         []() { return _("Increase Game Speed by 10×"); })},
   {KeyboardShortcut::kInGameSpeedReset,
    KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kGame},
                         keysym(SDLK_PAUSE, KMOD_SHIFT),
                         "game_speed_reset",
                         []() { return _("Reset Game Speed"); })},
   {KeyboardShortcut::kInGamePause,
    KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kGame},
                         keysym(SDLK_PAUSE),
                         "game_pause",
                         []() { return _("Pause"); })},
   {KeyboardShortcut::kInGameScrollToHQ,
    KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kGame},
                         keysym(SDLK_HOME),
                         "game_hq",
                         []() { return _("Scroll To Starting Field"); })},
   {KeyboardShortcut::kInGameChat,
    KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kGame},
                         keysym(SDLK_RETURN),
                         "game_chat",
                         []() { return _("Chat"); })},
   {KeyboardShortcut::kInGameSave,
    KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kGame},
                         keysym(SDLK_s, KMOD_CTRL),
                         "game_save",
                         []() { return _("Save Game"); })},
};

std::string to_string(const KeyboardShortcut id) {
	return shortcuts_.at(id).descname();
}

SDL_Keysym get_default_shortcut(const KeyboardShortcut id) {
	return shortcuts_.at(id).default_shortcut;
}

static void write_shortcut(const KeyboardShortcut id, const SDL_Keysym code) {
	set_config_int("keyboard_sym", shortcuts_.at(id).internal_name, code.sym);
	set_config_int("keyboard_mod", shortcuts_.at(id).internal_name, code.mod);
}

static bool shared_scope(const std::set<KeyboardShortcutInfo::Scope>& scopes, const KeyboardShortcutInfo& k) {
	if (scopes.count(KeyboardShortcutInfo::Scope::kGlobal) || k.scopes.count(KeyboardShortcutInfo::Scope::kGlobal)) {
		return true;
	}

	for (KeyboardShortcutInfo::Scope s : scopes) {
		if (k.scopes.count(s)) {
			return true;
		}
	}
	return false;
}

bool set_shortcut(const KeyboardShortcut id, const SDL_Keysym code, KeyboardShortcut* conflict) {
	const std::set<KeyboardShortcutInfo::Scope>& scopes = shortcuts_.at(id).scopes;

	for (auto& pair : shortcuts_) {
		if (pair.first != id && shared_scope(scopes, pair.second) && matches_shortcut(pair.first, code)) {
			*conflict = pair.first;
			return false;
		}
	}

	shortcuts_.at(id).current_shortcut = code;
	write_shortcut(id, code);
	return true;
}

SDL_Keysym get_shortcut(const KeyboardShortcut id) {
	return shortcuts_.at(id).current_shortcut;
}

static const std::map<SDL_Keycode, SDL_Keycode> kNumpadIdentifications = {
	{SDLK_KP_9, SDLK_PAGEUP},
	{SDLK_KP_8, SDLK_UP},
	{SDLK_KP_7, SDLK_HOME},
	{SDLK_KP_6, SDLK_RIGHT},
	{SDLK_KP_4, SDLK_LEFT},
	{SDLK_KP_3, SDLK_PAGEDOWN},
	{SDLK_KP_2, SDLK_DOWN},
	{SDLK_KP_1, SDLK_END},
	{SDLK_KP_0, SDLK_INSERT},
	{SDLK_KP_PERIOD, SDLK_DELETE},
	{SDLK_KP_ENTER, SDLK_RETURN},
	{SDLK_KP_MINUS, SDLK_MINUS},
	{SDLK_KP_PLUS, SDLK_PLUS},
};

bool matches_shortcut(const KeyboardShortcut id, const SDL_Keysym code) {
	return matches_shortcut(id, code.sym, code.mod);
}
bool matches_shortcut(const KeyboardShortcut id, SDL_Keycode code, const int mod) {
	const SDL_Keysym key = get_shortcut(id);

	const bool ctrl1 = key.mod & KMOD_CTRL;
	const bool shift1 = key.mod & KMOD_SHIFT;
	const bool alt1 = key.mod & KMOD_ALT;
	const bool gui1 = key.mod & KMOD_GUI;
	const bool ctrl2 = mod & KMOD_CTRL;
	const bool shift2 = mod & KMOD_SHIFT;
	const bool alt2 = mod & KMOD_ALT;
	const bool gui2 = mod & KMOD_GUI;

	if (ctrl1 != ctrl2 || shift1 != shift2 || alt1 != alt2 || gui1 != gui2) {
		return false;
	}

	if (key.sym == code) {
		return true;
	}

	// Some extra checks so we can identify keypad keys with their "normal" equivalents,
	// e.g. pressing '+' or numpad_'+' should always have the same effect

	if ((mod & KMOD_NUM) && ((key.sym >= SDLK_KP_0 && key.sym <= SDLK_KP_9) || (code >= SDLK_KP_0 && code <= SDLK_KP_9))) {
		// If numlock is on and a number was pressed, only compare the entered number value
		return (code >= SDLK_KP_0 && code <= SDLK_KP_9 && code == key.sym + SDLK_KP_0 - SDLK_0)
			|| (code >= SDLK_0 && code <= SDLK_9 && code == key.sym + SDLK_0 - SDLK_KP_0);
	}

	for (const auto& pair : kNumpadIdentifications) {
		if ((code == pair.first && key.sym == pair.second) || (code == pair.second && key.sym == pair.first)) {
			return true;
		}
	}

	return false;
}

std::string shortcut_string_for(const KeyboardShortcut id) {
	return shortcut_string_for(get_shortcut(id));
}

std::string shortcut_string_for(const SDL_Keysym sym) {
	std::vector<std::string> mods;
	if (sym.mod & KMOD_SHIFT) {
		mods.push_back(pgettext("hotkey", "Shift"));
	}
	if (sym.mod & KMOD_ALT) {
		mods.push_back(pgettext("hotkey", "Alt"));
	}
	if (sym.mod & KMOD_GUI) {
		mods.push_back(pgettext("hotkey", "GUI"));
	}
	if (sym.mod & KMOD_CTRL) {
		mods.push_back(pgettext("hotkey", "Ctrl"));
	}

	std::string result = SDL_GetKeyName(sym.sym);
	for (const std::string& m : mods) {
		result = (boost::format(_("%1$s+%2$s")) % m % result).str();
	}

	return richtext_escape(result);
}

void init_shortcuts(const bool force_defaults) {
	for (KeyboardShortcut k = KeyboardShortcut::k__Begin; k <= KeyboardShortcut::k__End;
	     k = static_cast<KeyboardShortcut>(static_cast<uint16_t>(k) + 1)) {
		shortcuts_.at(k).current_shortcut = get_default_shortcut(k);
		if (force_defaults) {
			write_shortcut(k, shortcuts_.at(k).current_shortcut);
		}
	}
	if (force_defaults) {
		return;
	}

	Section& ss = get_config_section("keyboard_sym");
	Section& sm = get_config_section("keyboard_mod");
	while (Section::Value* v = ss.get_next_val()) {
		for (auto& pair : shortcuts_) {
			if (pair.second.internal_name == v->get_name()) {
				pair.second.current_shortcut.sym = v->get_int();
				break;
			}
		}
	}
	while (Section::Value* v = sm.get_next_val()) {
		for (auto& pair : shortcuts_) {
			if (pair.second.internal_name == v->get_name()) {
				pair.second.current_shortcut.mod = v->get_int();
				break;
			}
		}
	}
}

void set_config_directory(const std::string& userconfigdir) {
	config_dir.reset(new RealFSImpl(userconfigdir));
	config_dir->ensure_directory_exists(".");
	log_info("Set configuration file: %s/%s\n", userconfigdir.c_str(), kConfigFile.c_str());
}

void read_config() {
	assert(config_dir != nullptr);
	g_options.read(kConfigFile.c_str(), "global", *config_dir);
}

void write_config() {
	assert(config_dir != nullptr);
	try {  //  overwrite the old config file
		g_options.write(kConfigFile.c_str(), true, *config_dir);
	} catch (const std::exception& e) {
		log_warn("could not save configuration: %s\n", e.what());
	} catch (...) {
		log_warn("could not save configuration");
	}
}
