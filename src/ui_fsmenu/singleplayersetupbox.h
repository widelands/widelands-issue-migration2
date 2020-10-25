/*
 * Copyright (C) 2002-2020 by the Widelands Development Team
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

#ifndef WL_UI_FSMENU_SINGLEPLAYERSETUPBOX_H
#define WL_UI_FSMENU_SINGLEPLAYERSETUPBOX_H

#include <vector>

#include "ui_fsmenu/playersetupbox.h"
#include "ui_fsmenu/singleplayerdropdown.h"

// horizontal group
class SinglePlayerActivePlayerGroup : public UI::Box {
public:
	SinglePlayerActivePlayerGroup(UI::Panel* const parent,
	                              int32_t const w,
	                              int32_t const h,
	                              PlayerSlot id,
	                              GameSettingsProvider* const settings);

	void update();
	void force_new_dimensions(float scale, uint32_t standard_element_height);

private:
	PlayerSlot id_;
	GameSettingsProvider* const settings_;
	UI::Button player_;
	SinglePlayerPlayerTypeDropdown player_type_;
	SinglePlayerTribeDropdown tribe_;
	SinglePlayerStartTypeDropdown start_type;
	SinglePlayerTeamDropdown teams_;
};

class SinglePlayerSetupBox : public PlayerSetupBox {

public:
	SinglePlayerSetupBox(UI::Panel* const parent,
	                     GameSettingsProvider* const settings,
	                     uint32_t standard_element_height,
	                     uint32_t padding);

	void force_new_dimensions(float scale, uint32_t standard_element_height);

protected:
	void update() override;
	void reset() override;
	void update_player_group(size_t index) override;

private:
	std::vector<SinglePlayerActivePlayerGroup*> active_player_groups_;  // not owned
};
#endif  // WL_UI_FSMENU_SINGLEPLAYERSETUPBOX_H
