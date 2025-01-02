/*
 * Copyright (C) 2010-2025 by the Widelands Development Team
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

#include "logic/cmd_delete_message.h"

#include "logic/game.h"
#include "logic/player.h"

namespace Widelands {

void CmdDeleteMessage::execute(Game& game) {
	game.get_player(player)->get_messages()->delete_message(message);
}
}  // namespace Widelands
