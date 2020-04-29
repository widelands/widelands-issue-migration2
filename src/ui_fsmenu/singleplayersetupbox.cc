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

SinglePlayerSetupBox::SinglePlayerSetupBox(UI::Panel* const parent,
                                           int32_t const x,
                                           int32_t const y,
                                           int32_t const w,
                                           int32_t const h,
                                           GameSettingsProvider* const settings,
                                           uint32_t buth)
   : UI::Box(parent, x, y, UI::Box::Vertical, w, h), settings_(settings) {

	single_player_player_groups.resize(kMaxPlayers);
	for (PlayerSlot i = 0; i < single_player_player_groups.size(); ++i) {
		single_player_player_groups.at(i) =
		   new SinglePlayerPlayerGroup(this, 0 /*get_w() - UI::Scrollbar::kSize*/, buth, i, settings);
		add(single_player_player_groups.at(i));
	}
}

SinglePlayerPlayerGroup::SinglePlayerPlayerGroup(UI::Panel* const parent,
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
}

void SinglePlayerPlayerGroup::on_gamesettings_updated(const NoteGameSettings& note) {
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
	case NoteGameSettings::Action::kUser:
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

/// Refresh all user interfaces
void SinglePlayerPlayerGroup::update() {
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
