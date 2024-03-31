/*
 * Copyright (C) 2024 by the Widelands Development Team
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

#ifndef WL_COMMANDS_CMD_SHIP_SET_DESTINATION_H
#define WL_COMMANDS_CMD_SHIP_SET_DESTINATION_H

#include "commands/command.h"
#include "logic/detected_port_space.h"

namespace Widelands {

struct CmdShipSetDestination : public PlayerCommand {
	CmdShipSetDestination() = default;  // For savegame loading
	CmdShipSetDestination(const Time& t, PlayerNumber const p, Serial s, Serial dest)
	   : PlayerCommand(t, p), serial_(s), destination_object_(dest) {
	}
	CmdShipSetDestination(const Time& t,
	                      PlayerNumber const p,
	                      Serial s,
	                      const DetectedPortSpace& dest)
	   : PlayerCommand(t, p), serial_(s), destination_coords_(dest.serial) {
	}

	void write(FileWrite&, EditorGameBase&, MapObjectSaver&) override;
	void read(FileRead&, EditorGameBase&, MapObjectLoader&) override;

	[[nodiscard]] QueueCommandTypes id() const override {
		return QueueCommandTypes::kShipSetDestination;
	}

	explicit CmdShipSetDestination(StreamRead&);

	void execute(Game&) override;
	void serialize(StreamWrite&) override;

private:
	Serial serial_{0U};
	Serial destination_object_{0U};
	Serial destination_coords_{0U};
};

}  // namespace Widelands

#endif  // end of include guard: WL_COMMANDS_CMD_SHIP_SET_DESTINATION_H
