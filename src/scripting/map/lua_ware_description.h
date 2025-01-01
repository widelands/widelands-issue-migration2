/*
 * Copyright (C) 2006-2025 by the Widelands Development Team
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

#ifndef WL_SCRIPTING_MAP_LUA_WARE_DESCRIPTION_H
#define WL_SCRIPTING_MAP_LUA_WARE_DESCRIPTION_H

#include "logic/map_objects/tribes/ware_descr.h"
#include "scripting/map/lua_map_object_description.h"

namespace LuaMaps {

class LuaWareDescription : public LuaMapObjectDescription {
public:
	LUNA_CLASS_HEAD(LuaWareDescription);

	~LuaWareDescription() override = default;

	LuaWareDescription() = default;
	explicit LuaWareDescription(const Widelands::WareDescr* const waredescr)
	   : LuaMapObjectDescription(waredescr) {
	}
	explicit LuaWareDescription(lua_State* L) : LuaMapObjectDescription(L) {
	}

	CLANG_DIAG_RESERVED_IDENTIFIER_OFF
	void __persist(lua_State* L) override;
	void __unpersist(lua_State* L) override;
	CLANG_DIAG_RESERVED_IDENTIFIER_ON

	/*
	 * Properties
	 */

	/*
	 * Lua methods
	 */
	int consumers(lua_State*);
	int is_construction_material(lua_State*);
	int producers(lua_State*);

	/*
	 * C methods
	 */

private:
	CASTED_GET_DESCRIPTION(WareDescr)
};

}  // namespace LuaMaps

#endif
