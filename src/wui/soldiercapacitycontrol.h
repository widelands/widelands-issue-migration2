/*
 * Copyright (C) 2002-2025 by the Widelands Development Team
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

#ifndef WL_WUI_SOLDIERCAPACITYCONTROL_H
#define WL_WUI_SOLDIERCAPACITYCONTROL_H

class InteractiveBase;

namespace UI {
class Panel;
}  // namespace UI

namespace Widelands {
class MapObject;
}  // namespace Widelands

UI::Panel* create_soldier_capacity_control(UI::Panel& parent,
                                           InteractiveBase&,
                                           Widelands::MapObject& building_or_ship);

#endif  // end of include guard: WL_WUI_SOLDIERCAPACITYCONTROL_H
