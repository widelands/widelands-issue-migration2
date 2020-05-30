/*
 * Copyright (C) 2020-2020 by the Widelands Development Team
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

#ifndef WL_NETWORK_NET_ADDONS_H
#define WL_NETWORK_NET_ADDONS_H

#include <set>

#include <curl/curl.h>

#include "logic/addons.h"

// The add-on related networking functions defined here use the cURL lib.
// Pro: I created a functional dummy server with no knowledge of the metaserver backend ;)
// Con: Additional dependency – this is the only place in our code where libcurl is used

struct NetAddons {
	NetAddons() : curl_(nullptr) {
	}
	~NetAddons();

	// Fetch the list of all available add-ons from the server
	std::vector<AddOnInfo> refresh_remotes();

	// Requests the file with the given name server and downloads it into thr given location.
	void download_addon_file(const std::string& name, const std::string& save_as);

	// Requests the MO file for the given add-on and locale from the server,
	// downloads it into a temporary location (e.g. ~/.widelands/temp/nds.mo.tmp),
	// and returns the canonical path to the downloaded file.
	// The temp file's filename is guaranteed to be in the format
	// "nds.mo.tmp" (where 'nds' is the language's abbreviation).
	// Returns "" on failure.
	std::string download_i18n(const std::string& addon, const std::string& locale);

private:
	// Open the connection if it was not open yet; throws an error if this fails
	void init();

	CURL* curl_;
};

#endif  // end of include guard: WL_NETWORK_NET_ADDONS_H
