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

#ifndef WL_COMMANDS_CMD_MESSAGE_SET_STATUS_READ_H
#define WL_COMMANDS_CMD_MESSAGE_SET_STATUS_READ_H

#include "commands/command.h"

namespace Widelands {

struct CmdMessageSetStatusRead : public PlayerMessageCommand {
	CmdMessageSetStatusRead() = default;
	CmdMessageSetStatusRead(const Time& t, const PlayerNumber p, const MessageId& i)
	   : PlayerMessageCommand(t, p, i) {
	}

	[[nodiscard]] QueueCommandTypes id() const override {
		return QueueCommandTypes::kMessageSetStatusRead;
	}

	explicit CmdMessageSetStatusRead(StreamRead& des) : PlayerMessageCommand(des) {
	}

	void execute(Game&) override;
	void serialize(StreamWrite&) override;
};

}  // namespace Widelands

#endif  // end of include guard: WL_COMMANDS_CMD_MESSAGE_SET_STATUS_READ_H
