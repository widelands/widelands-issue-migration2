/*
 * Copyright (C) 2002-2022 by the Widelands Development Team
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

#ifndef WL_EDITOR_TOOLS_PLACE_IMMOVABLE_TOOL_H
#define WL_EDITOR_TOOLS_PLACE_IMMOVABLE_TOOL_H

#include "editor/tools/delete_immovable_tool.h"
#include "editor/tools/multi_select.h"

/**
 * This places immovables on the map
 */
struct EditorPlaceImmovableTool : public EditorTool, public MultiSelect {
	explicit EditorPlaceImmovableTool(EditorDeleteImmovableTool& tool) : EditorTool(tool, tool) {
	}

	int32_t handle_click_impl(const Widelands::NodeAndTriangle<>& center,
	                          EditorInteractive& eia,
	                          EditorActionArgs* args,
	                          Widelands::Map* map) override;

	int32_t handle_undo_impl(const Widelands::NodeAndTriangle<>& center,
	                         EditorInteractive& eia,
	                         EditorActionArgs* args,
	                         Widelands::Map* map) override;

	EditorActionArgs format_args_impl(EditorInteractive& parent) override;

	const Image* get_sel_impl() const override {
		return g_image_cache->get("images/wui/editor/fsel_editor_place_immovable.png");
	}

        WindowID get_window_id() override {
                return WindowID::Immovables;
        }

        bool save_configuration_impl(ToolConf& conf, EditorInteractive& parent) override;
        void load_configuration(const ToolConf& conf) override;
        std::string format_conf_string_impl(EditorInteractive& parent, const ToolConf& conf) override;

};

#endif  // end of include guard: WL_EDITOR_TOOLS_PLACE_IMMOVABLE_TOOL_H
