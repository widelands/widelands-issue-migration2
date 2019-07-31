/*
 * Copyright (C) 2019 by the Widelands Development Team
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

#include "graphic/animation/spritesheet_animation.h"

#include <cassert>
#include <cstdio>
#include <limits>
#include <memory>

#include <boost/algorithm/string/replace.hpp>
#include <boost/format.hpp>

#include "base/macros.h"
#include "graphic/graphic.h"
#include "graphic/image.h"
#include "graphic/playercolor.h"
#include "graphic/texture.h"
#include "io/filesystem/filesystem.h"
#include "io/filesystem/layered_filesystem.h"
#include "logic/game_data_error.h"
#include "scripting/lua_table.h"

/*
==============================================================================

SpriteSheetAnimation::MipMapEntry IMPLEMENTATION

==============================================================================
*/

SpriteSheetAnimation::SpriteSheetMipMapEntry::SpriteSheetMipMapEntry(const std::string& file, int init_rows, int init_columns)
   : Animation::MipMapEntry(), sheet_file(file), sheet(nullptr), playercolor_mask_sheet(nullptr), rows(init_rows), columns(init_columns), w(0), h(0), playercolor_mask_sheet_file("") {

	assert(g_fs->file_exists(file));

	playercolor_mask_sheet_file = file;
	boost::replace_last(playercolor_mask_sheet_file, ".png", "_pc.png");
	if (g_fs->file_exists(playercolor_mask_sheet_file)) {
		has_playercolor_masks = true;
	} else {
		playercolor_mask_sheet_file = "";
	}
}


// Loads the graphics if they are not yet loaded.
void SpriteSheetAnimation::SpriteSheetMipMapEntry::ensure_graphics_are_loaded() const {
	if (sheet == nullptr) {
		const_cast<SpriteSheetMipMapEntry*>(this)->load_graphics();
	}
}

// Load the needed graphics from disk.
void SpriteSheetAnimation::SpriteSheetMipMapEntry::load_graphics() {
	sheet = g_gr->images().get(sheet_file);

	if (!playercolor_mask_sheet_file.empty()) {
		playercolor_mask_sheet = g_gr->images().get(playercolor_mask_sheet_file);

		if (sheet->width() != playercolor_mask_sheet->width()) {
			throw Widelands::GameDataError(
			   "animation sprite sheet has width %d but playercolor mask sheet has width %d. The sheet's image is %s",
			   sheet->width(), playercolor_mask_sheet->width(), sheet_file.c_str());
		}
		if (sheet->height() != playercolor_mask_sheet->height()) {
			throw Widelands::GameDataError(
			   "animation sprite sheet has height %d but playercolor mask sheet has height %d. The sheet's image is %s",
			   sheet->height(), playercolor_mask_sheet->height(), sheet_file.c_str());
		}
	}

	// Frame width and height
	w = sheet->width() / columns;
	h = sheet->height() / rows;
}

void SpriteSheetAnimation::SpriteSheetMipMapEntry::blit(uint32_t idx,
                                           const Rectf& source_rect,
                                           const Rectf& destination_rect,
                                           const RGBColor* clr,
                                           Surface* target) const {
	assert(sheet != nullptr);
	assert(target);
	assert(static_cast<int>(idx) <= columns * rows);

	const int column = idx % columns;
	const int row = idx / rows;

	Rectf frame_rect(source_rect.x + column * width(), source_rect.y + row * height(),
					 source_rect.w, source_rect.h);

	if (!has_playercolor_masks || clr == nullptr) {
		target->blit(destination_rect, *sheet, frame_rect, 1., BlendMode::UseAlpha);
	} else {
		assert(playercolor_mask_sheet != nullptr);
		target->blit_blended(
		   destination_rect, *sheet, *playercolor_mask_sheet, frame_rect, *clr);
	}
}

int SpriteSheetAnimation::SpriteSheetMipMapEntry::width() const {
	return w;
}
int SpriteSheetAnimation::SpriteSheetMipMapEntry::height() const {
	return h;
}

/*
==============================================================================

SpriteSheetAnimation IMPLEMENTATION

==============================================================================
*/

SpriteSheetAnimation::SpriteSheetAnimation(const LuaTable& table, const std::string& basename)
   : Animation(table) {
	assert(table.has_key("columns"));
	try {
		// Get image files
		if (basename.empty() || !table.has_key("directory")) {
			throw Widelands::GameDataError(
			   "Animation did not define both a basename and a directory for its sprite sheet file");
		}
		const std::string directory = table.get_string("directory");

		// Frames, rows and columns
		nr_frames_ = table.get_int("frames");
		rows_ = table.get_int("rows");
		columns_ = table.get_int("columns");

		// Look for a file for the given scale, and if we have it, add a mipmap entry for it.
		auto add_scale = [this, basename, directory](
							float scale_as_float, const std::string& scale_as_string) {
			const std::string path =
			   directory + g_fs->file_separator() + basename + scale_as_string + ".png";
			if (g_fs->file_exists(path)) {
				mipmaps_.insert(std::make_pair(
				   scale_as_float, std::unique_ptr<SpriteSheetMipMapEntry>(new SpriteSheetMipMapEntry(path, rows_, columns_))));
			}
		};
		add_scale(0.5f, "_0.5");
		add_scale(1.0f, "_1");
		add_scale(2.0f, "_2");
		add_scale(4.0f, "_4");

		if (mipmaps_.count(1.0f) == 0) {
			// There might be only 1 scale
			add_scale(1.0f, "");
			if (mipmaps_.count(1.0f) == 0) {
				// No files found at all
				throw Widelands::GameDataError(
				   "Animation in directory '%s' with basename '%s' has no sprite sheet for mandatory "
				   "scale '1' in mipmap - supported scales are: 0.5, 1, 2, 4",
				   directory.c_str(), basename.c_str());
			}
		}

		// Perform some checks to make sure that the data is complete and consistent
		const SpriteSheetMipMapEntry& first = dynamic_cast<const SpriteSheetMipMapEntry&>(*mipmaps_.begin()->second.get());
		if (table.has_key("fps") && nr_frames_ == 1) {
				throw Widelands::GameDataError("Animation with one frame in sprite sheet %s must not have 'fps'",
				                               first.sheet_file.c_str());
		}

		if (representative_frame() < 0 || representative_frame() > nr_frames_ - 1) {
			throw Widelands::GameDataError("Animation has %d as its representative frame, but the frame indices "
			                 "available are 0 - %d",
			                 representative_frame(), nr_frames_ - 1);
		}

		if (rows_ * columns_ < nr_frames_) {
			throw Widelands::GameDataError("Animation has %d frames, which does not fit into %d rows x %d columns",
										   nr_frames_, rows_, columns_);
		}

		const bool should_have_playercolor = first.has_playercolor_masks;
		for (const auto& mipmap : mipmaps_) {
			if (first.has_playercolor_masks != should_have_playercolor) {
				throw Widelands::GameDataError(
				   "Mismatched existence of player colors in animation table for scales %.2f and %.2f",
				   static_cast<double>(mipmaps_.begin()->first), static_cast<double>(mipmap.first));
			}
		}
		if (mipmaps_.count(1.0f) != 1) {
			throw Widelands::GameDataError(
			   "All animations must provide images for the neutral scale (1.0)");
		}
	} catch (const LuaError& e) {
		throw Widelands::GameDataError("Error in animation table: %s", e.what());
	}
}

std::vector<const Image*> SpriteSheetAnimation::images(float) const {
	// We only need to implement this if we add compressed spritemaps, or maybe for usage in a test
	NEVER_HERE();
}

std::vector<const Image*> SpriteSheetAnimation::pc_masks(float) const {
	// We only need to implement this if we add compressed spritemaps, or maybe for usage in a test
	NEVER_HERE();
}

const Image* SpriteSheetAnimation::representative_image(const RGBColor* clr) const {
	const SpriteSheetMipMapEntry& mipmap = dynamic_cast<const SpriteSheetMipMapEntry&>(mipmap_entry(1.0f));
	const int column = representative_frame() % columns_;
	const int row = representative_frame() / rows_;
	const int w = width();
	const int h = height();

	Texture* rv = new Texture(w, h);
	Rectf rect(Vector2f::zero(), w, h);
	if (mipmap.has_playercolor_masks && clr) {
		rv->fill_rect(rect, RGBAColor(0, 0, 0, 0));
		rv->blit_blended(Rectf(column * w, row * h, w, h), *mipmap.sheet, *mipmap.playercolor_mask_sheet, rect, *clr);
	} else {
		rv->blit(Rectf(column * w, row * h, w, h), *mipmap.sheet, rect, 1., BlendMode::Copy);
	}
	return rv;
}

// NOCOM Barbarian carriers are not walking smoothly
