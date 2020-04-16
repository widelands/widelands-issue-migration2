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

#ifndef WL_EDITOR_TOOLS_INCREASE_HEIGHT_TOOL_H
#define WL_EDITOR_TOOLS_INCREASE_HEIGHT_TOOL_H

#include "editor/tools/decrease_height_tool.h"
#include "editor/tools/set_height_tool.h"

///  Increases the height of a field by a value.
struct EditorIncreaseHeightTool : public EditorTool {
	EditorIncreaseHeightTool(EditorDecreaseHeightTool& the_decrease_tool,
	                         EditorSetHeightTool& the_set_tool)
	   : EditorTool(the_decrease_tool, the_set_tool),
	     decrease_tool_(the_decrease_tool),
	     set_tool_(the_set_tool),
	     change_by_(1) {
	}

	int32_t handle_click_impl(const Widelands::NodeAndTriangle<>& center,
	                          EditorInteractive& parent,
	                          EditorActionArgs* args,
	                          Widelands::Map* map) override;

	int32_t handle_undo_impl(const Widelands::NodeAndTriangle<>& center,
	                         EditorInteractive& parent,
	                         EditorActionArgs* args,
	                         Widelands::Map* map) override;

	EditorActionArgs format_args_impl(EditorInteractive& parent) override;

	const Image* get_sel_impl() const override {
		return g_gr->images().get("images/wui/editor/fsel_editor_increase_height.png");
	}

	int32_t get_change_by() const {
		return change_by_;
	}
	void set_change_by(const int32_t n) {
		change_by_ = n;
	}

	EditorDecreaseHeightTool& decrease_tool() const {
		return decrease_tool_;
	}
	EditorSetHeightTool& set_tool() const {
		return set_tool_;
	}

private:
	EditorDecreaseHeightTool& decrease_tool_;
	EditorSetHeightTool& set_tool_;
	int32_t change_by_;
};

#endif  // end of include guard: WL_EDITOR_TOOLS_INCREASE_HEIGHT_TOOL_H
