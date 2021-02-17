/*
 * Copyright (C) 2020-2021 by the Widelands Development Team
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

#include "logic/addons.h"

namespace AddOns {

struct NetAddons {
	NetAddons() : initialized_(false), client_socket_(0) {
	}
	~NetAddons();

	// Fetch the list of all available add-ons from the server
	std::vector<AddOnInfo> refresh_remotes();

	using CallbackFn = std::function<void(const std::string&, long)>;

	// Downloads the add-on with the given name (e.g. "cool_feature.wad")
	// from the server and downloads it to the given canonical location.
	void download_addon(const std::string& name, const std::string& save_as, const CallbackFn&);

	// Requests the MO files for the given add-on (cool_feature.wad) from the server and
	// downloads them into the given temporary location (e.g. ~/.widelands/temp/some_dir).
	// The filename of the created MO files is guaranteed to be in the format
	// "nds.mo.tmp" (where 'nds' is the language's abbreviation).
	void download_i18n(const std::string& addon, const std::string& directory, const CallbackFn&, const CallbackFn&);

	// Download the given screenshot for the given add-on
	std::string download_screenshot(const std::string& addon, const std::string& screenie);

	// How the user voted the add-on (1-10). Returns 0 for not votes, <0 for access denied.
	int get_vote(const std::string& addon);
	void vote(const std::string& addon, unsigned vote);
	void comment(const AddOnInfo& addon, const std::string& message);

	void set_login(const std::string& username, const std::string& password);

private:
	friend struct CrashGuard;

	// Open the connection if it was not open yet; throws an error if this fails
	void init(const std::string& username = std::string(), const std::string& password = std::string());
	void quit_connection();

	// Set the URL (whitespace-safe) and adjust the timeout values.
	// void set_url_and_timeout(std::string);

	// Read a '\n'-terminated string from the socket. The terminator is not part of the result.
	std::string read_line();
	void read_file(long length, const std::string& out);
	void check_endofstream();

	bool initialized_;
	int client_socket_;
};

}  // namespace AddOns

#endif  // end of include guard: WL_NETWORK_NET_ADDONS_H
