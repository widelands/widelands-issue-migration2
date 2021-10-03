/*
 * Copyright (C) 2006-2021 by the Widelands Development Team
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifndef WL_BUILD_INFO_H
#define WL_BUILD_INFO_H

#include <boost/format.hpp>

#include "base/i18n.h"

constexpr uint16_t kWidelandsCopyrightStart = 2002;
// If the following line is changed, the corresponding regex in 'utils/update_copyright.py' will
// also need updating.
constexpr uint16_t kWidelandsCopyrightEnd = 2021;

/// \return the build id which is automagically created from the revision number
/// or the VERSION file
const std::string& build_id();

/// \return the build type, which is set during compile time (either manually
/// or to a default value)
const std::string& build_type();

inline std::string version_string() {
	return (boost::format(_("Version %1$s (%2$s)")) % build_id() % build_type()).str();
}

#endif  // end of include guard: WL_BUILD_INFO_H
