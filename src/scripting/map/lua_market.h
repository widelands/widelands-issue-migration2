/*
 * Copyright (C) 2006-2024 by the Widelands Development Team
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

#ifndef WL_SCRIPTING_MAP_LUA_MARKET_H
#define WL_SCRIPTING_MAP_LUA_MARKET_H

#include "logic/map_objects/tribes/market.h"
#include "scripting/map/lua_building.h"

namespace LuaMaps {

class LuaMarket : public LuaBuilding {
public:
	LUNA_CLASS_HEAD(LuaMarket);

	LuaMarket() = default;
	explicit LuaMarket(Widelands::Market& mo) : LuaBuilding(mo) {
	}
	explicit LuaMarket(lua_State* L) : LuaBuilding(L) {
	}
	~LuaMarket() override = default;

	/*
	 * Properties
	 */

	/*
	 * Lua Methods
	 */
	int propose_trade(lua_State* L);

	/*
	 * C Methods
	 */
	CASTED_GET(Market)
};

}  // namespace LuaMaps

#endif
