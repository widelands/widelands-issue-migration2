/*
 * Copyright (C) 2002-2024 by the Widelands Development Team
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

#include "base/crypto.h"

#include <sha1.h>

namespace crypto {

std::string MD5Checksum::str() const {
	std::string result(size() * 2, '?');
	for (size_t i = 0; i < size(); ++i) {
		snprintf(result.data() + 2 * i, 3, "%02x", at(i));
	}
	return result;
}

bool MD5Checksum::operator==(const MD5Checksum& other) const {
	return memcmp(data(), other.data(), size()) == 0;
}

MD5Checksummer::MD5Checksummer() {
	reset();
}

void MD5Checksummer::reset() {
	MD5Init(&context_);
}

void MD5Checksummer::data(const void* data, size_t len) {
	MD5Update(&context_, reinterpret_cast<const uint8_t*>(data), len);
}

std::string MD5Checksummer::finish_checksum_str() {
	std::string result(MD5_DIGEST_STRING_LENGTH - 1 /* correct for implicit null terminator */, '?');
	MD5End(&context_, result.data());
	reset();
	return result;
}

MD5Checksum MD5Checksummer::finish_checksum_raw() {
	MD5Checksum result;
	MD5Final(result.data(), &context_);
	reset();
	return result;
}

std::string sha1(const void* data, size_t len) {
	std::string result(
	   SHA1_DIGEST_STRING_LENGTH - 1 /* correct for implicit null terminator */, '?');
	SHA1_CTX context;
	SHA1Init(&context);
	SHA1Update(&context, reinterpret_cast<const uint8_t*>(data), len);
	SHA1End(&context, result.data());
	return result;
}

}  // namespace crypto
