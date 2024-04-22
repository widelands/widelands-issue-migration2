/*
 * Copyright (C) 2004-2024 by the Widelands Development Team
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

#ifndef WL_COMMANDS_CMD_PICK_CUSTOM_STARTING_POSITION_H
#define WL_COMMANDS_CMD_PICK_CUSTOM_STARTING_POSITION_H

#include "commands/command.h"
#include "logic/widelands_geometry.h"

namespace Widelands {

struct CmdPickCustomStartingPosition : PlayerCommand {
	CmdPickCustomStartingPosition(const Time& t, PlayerNumber p, const Coords& c)
	   : PlayerCommand(t, p), coords_(c) {
	}

	[[nodiscard]] QueueCommandTypes id() const override {
		return QueueCommandTypes::kPickCustomStartingPosition;
	}

	void execute(Game& game) override;

	explicit CmdPickCustomStartingPosition(StreamRead& des);
	void serialize(StreamWrite& ser) override;

	CmdPickCustomStartingPosition() = default;
	void write(FileWrite&, EditorGameBase&, MapObjectSaver&) override;
	void read(FileRead&, EditorGameBase&, MapObjectLoader&) override;

private:
	Coords coords_;
};

}  // namespace Widelands

#endif  // end of include guard: WL_COMMANDS_CMD_PICK_CUSTOM_STARTING_POSITION_H
