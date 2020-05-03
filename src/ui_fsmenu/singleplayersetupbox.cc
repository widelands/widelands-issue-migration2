#include "singleplayersetupbox.h"

#include <memory>

#include <boost/algorithm/string.hpp>

#include "ai/computer_player.h"
#include "base/i18n.h"
#include "base/log.h"
#include "base/wexception.h"
#include "graphic/graphic.h"
#include "graphic/playercolor.h"
#include "logic/game.h"
#include "logic/game_settings.h"
#include "logic/map_objects/tribes/tribe_basic_info.h"
#include "logic/player.h"
#include "map_io/map_loader.h"
#include "ui_basic/button.h"
#include "ui_basic/dropdown.h"
#include "ui_basic/mouse_constants.h"
#include "ui_basic/scrollbar.h"
#include "ui_basic/textarea.h"

#define AI_NAME_PREFIX "ai" AI_NAME_SEPARATOR

SinglePlayerActivePlayerSetupBox::SinglePlayerActivePlayerSetupBox(
   UI::Panel* const parent,
   int32_t const x,
   int32_t const y,
   int32_t const w,
   int32_t const h,
   GameSettingsProvider* const settings,
   uint32_t buth)
   : UI::Box(parent, x, y, UI::Box::Vertical, w, h),
     title_(this,
            0,
            0,
            0,
            0,
            _("Players"),
            UI::Align::kRight,
            g_gr->styles().font_style(UI::FontStyle::kFsGameSetupHeadings)),
     settings_(settings) {
	add(&title_, Resizing::kAlign, UI::Align::kCenter);
	active_player_groups.resize(kMaxPlayers);
	for (PlayerSlot i = 0; i < active_player_groups.size(); ++i) {
		active_player_groups.at(i) = new SinglePlayerActivePlayerGroup(
		   this, 0 /*get_w() - UI::Scrollbar::kSize*/, buth, i, settings);
		add(active_player_groups.at(i));
	}
}

SinglePlayerActivePlayerGroup::SinglePlayerActivePlayerGroup(UI::Panel* const parent,
                                                             int32_t const w,
                                                             int32_t const h,
                                                             PlayerSlot id,
                                                             GameSettingsProvider* const settings)
   : UI::Box(parent, 0, 0, UI::Box::Horizontal, w, h),
     id_(id),
     settings_(settings),
     player(this,
            "player",
            0,
            0,
            h,
            h,
            UI::ButtonStyle::kFsMenuSecondary,
            playercolor_image(id, "images/players/player_position_menu.png"),
            (boost::format(_("Player %u")) % static_cast<unsigned int>(id_ + 1)).str(),
            UI::Button::VisualState::kFlat),
     player_type(this,
                 (boost::format("dropdown_type%d") % static_cast<unsigned int>(id)).str(),
                 0,
                 0,
                 h,
                 h,
                 settings,
                 id),
     tribe_(this,
            (boost::format("dropdown_type%d") % static_cast<unsigned int>(id)).str(),
            0,
            0,
            h,
            h,
            settings,
            id),
     start_type(this,
                (boost::format("dropdown_init%d") % static_cast<unsigned int>(id)).str(),
                0,
                0,
                4 * h,
                h,
                settings,
                id),
     teams_(this,
            (boost::format("dropdown_team%d") % static_cast<unsigned int>(id)).str(),
            0,
            0,
            h,
            h,
            settings,
            id) {

	add_space(0);
	add(&player);
	add(player_type.get_dropdown());
	add(tribe_.get_dropdown());
	add(start_type.get_dropdown());
	add(teams_.get_dropdown());
	add_space(0);

	subscriber_ = Notifications::subscribe<NoteGameSettings>(
	   [this](const NoteGameSettings& note) { on_gamesettings_updated(note); });

	player.set_disable_style(UI::ButtonDisableStyle::kFlat);
	player.set_enabled(false);
}

void SinglePlayerActivePlayerGroup::on_gamesettings_updated(const NoteGameSettings& note) {
	if (settings_->settings().players.empty()) {
		// No map/savegame yet
		return;
	}
	switch (note.action) {
	case NoteGameSettings::Action::kMap:
		// We don't care about map updates, since we receive enough notifications for the
		// slots. JM: maybe need to change this back...
		update();
		break;
	case NoteGameSettings::Action::kPlayer:  // JM: was kUser for multiplayer...
		// We might have moved away from a slot, so we need to update the previous slot too.
		// Since we can't track the slots here, we just update everything.
		update();
		break;
	default:
		if (id_ == note.position ||
		    (id_ < settings_->settings().players.size() &&
		     settings_->settings().players.at(id_).state == PlayerSettings::State::kShared)) {
			update();
		}
	}
}

void SinglePlayerActivePlayerGroup::update() {
	log("SinglePlayerActivePlayerGroup::update\n");
	const GameSettings& settings = settings_->settings();
	if (id_ >= settings.players.size()) {
		set_visible(false);
		return;
	}
	const PlayerSettings& player_setting = settings.players[id_];
	player_type.rebuild();
	set_visible(true);

	if (player_setting.state == PlayerSettings::State::kClosed ||
	    player_setting.state == PlayerSettings::State::kOpen) {

		teams_.set_visible(false);
		teams_.set_enabled(false);

		tribe_.set_visible(false);
		tribe_.set_enabled(false);

		start_type.set_visible(false);
		start_type.set_enabled(false);
	} else {  // kHuman, kShared, kComputer
		tribe_.rebuild();
		start_type.rebuild();
		teams_.rebuild();
	}

	// Trigger update for the other players for shared_in mode when slots open and close
	//	if (last_state_ != player_setting.state) {
	//		last_state_ = player_setting.state;
	//		for (PlayerSlot slot = 0; slot < settings_->settings().players.size(); ++slot) {
	//			if (slot != id_) {
	//				n->set_player_state(slot, settings.players[slot].state);
	//			}
	//		}
	//	}
}

SinglePlayerPossiblePlayerGroup::SinglePlayerPossiblePlayerGroup(
   UI::Panel* const parent,
   int32_t const w,
   int32_t const h,
   PlayerSlot id,
   GameSettingsProvider* const settings)
   : UI::Box(parent, 0, 0, UI::Box::Horizontal, w, h),
     id_(id),
     role_(this,
           (boost::format("dropdown_slot%d") % static_cast<unsigned int>(id)).str(),
           0,
           0,
           h,
           h,
           settings,
           id),
     // Name needs to be initialized after the dropdown, otherwise the layout function will
     // crash.
     name_(this, 0, 0, 50, 50),
     settings_(settings) {
	add(role_.get_dropdown());
	add(&name_, UI::Box::Resizing::kAlign, UI::Align::kCenter);

	name_.set_text(settings->settings().players.at(id).name);

	subscriber_ = Notifications::subscribe<NoteGameSettings>(
	   [this](const NoteGameSettings& note) { update(); });
}
void SinglePlayerPossiblePlayerGroup::update() {
	role_.rebuild();
}

SinglePlayerPossiblePlayerSetupBox::SinglePlayerPossiblePlayerSetupBox(
   UI::Panel* const parent,
   int32_t const x,
   int32_t const y,
   int32_t const w,
   int32_t const h,
   GameSettingsProvider* const settings,
   uint32_t buth)
   : UI::Box(parent, x, y, UI::Box::Vertical, w, h),
     title_(this,
            0,
            0,
            0,
            0,
            _("Available Players"),
            UI::Align::kRight,
            g_gr->styles().font_style(UI::FontStyle::kFsGameSetupHeadings)),
     settings_(settings) {
	add(&title_, Resizing::kAlign, UI::Align::kCenter);

	subscriber_ = Notifications::subscribe<NoteGameSettings>(
	   [this](const NoteGameSettings& note) { update(); });
}
void SinglePlayerPossiblePlayerSetupBox::update() {
	const GameSettings& settings = settings_->settings();
	const size_t number_of_users = settings.players.size();
	log("number of users: %d\n", settings.users.size());
	log("number of players: %d\n", settings.players.size());
	// Update / initialize client groups

	possible_player_groups.resize(number_of_users);
	for (uint32_t i = 0; i < number_of_users; ++i) {
		if (!possible_player_groups.at(i)) {
			possible_player_groups.at(i) =
			   new SinglePlayerPossiblePlayerGroup(this, 100, 50, i, settings_);
			add(possible_player_groups.at(i), UI::Box::Resizing::kFullSize);
			//         possible_player_groups.at(i)->layout();
			possible_player_groups.at(i)->set_visible(settings.players.at(i).state ==
			                                          PlayerSettings::State::kHuman);
		}
	}
}

SinglePlayerSetupBox::SinglePlayerSetupBox(UI::Panel* const parent,
                                           int32_t const x,
                                           int32_t const y,
                                           int32_t const w,
                                           int32_t const h,
                                           GameSettingsProvider* const settings,
                                           uint32_t buth)
   : UI::Box(parent, x, y, UI::Box::Horizontal, w, h),
     inactive_players(this, x, y, w, h, settings, buth),
     active_players_setup(this, x, y, w, h, settings, buth) {
	add(&inactive_players);
	add_space(50);
	add(&active_players_setup);
}
