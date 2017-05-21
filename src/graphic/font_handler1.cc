/*
 * Copyright (C) 2002-2017 by the Widelands Development Team
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

#include "graphic/font_handler1.h"

#include <memory>

#include <boost/lexical_cast.hpp>

#include "graphic/text/rt_render.h"
#include "graphic/text/texture_cache.h"

namespace {

// The size of the richtext surface cache in bytes. All work that the richtext
// renderer does is / cached in this cache until it overflows. The idea is that
// this is big enough to cache the text that is used on a typical screen - so
// that we do not need to lay out text every frame. Last benchmarked at r7712,
// 30 MB was enough to cache texts for many frames (> 1000), while it is
// quickly overflowing in the map selection menu.
// This might need reevaluation if this font handler is used for more stuff.
constexpr uint32_t kTextureCacheSize = 30 << 20;  // shifting converts to MB
}  // namespace

namespace UI {

// Utility class to render a rich text string. The returned string is cached in
// the ImageCache, so repeated calls to render with the same arguments should not
// be a problem.
class FontHandler1 : public IFontHandler1 {
private:
	// A transient cache for the generated rendered texts
	class RenderCache : public TransientCache<RenderedText> {
	public:
		RenderCache(uint32_t max_size_in_bytes) : TransientCache<RenderedText>(max_size_in_bytes) {
		}

		std::shared_ptr<const RenderedText> insert(const std::string& hash,
		                                           std::shared_ptr<const RenderedText> entry) {
			// For the size calculation, we estimate that the member variables of each RenderedRect
			// take up ca. 13 * 32 bytes. It's all pointers or combinations of basic data types, so the
			// size requirement is pretty constant.
			constexpr uint32_t kRenderedRectSize = 13 * 32;
			return TransientCache<RenderedText>::insert(
			   hash, entry, entry->rects.size() * kRenderedRectSize);
		}
	};

public:
	FontHandler1(ImageCache* image_cache, const std::string& locale)
	   : texture_cache_(new TextureCache(kTextureCacheSize)),
	     render_cache_(new RenderCache(kTextureCacheSize)),
	     fontsets_(),
	     fontset_(fontsets_.get_fontset(locale)),
	     rt_renderer_(new RT::Renderer(image_cache, texture_cache_.get(), fontsets_)),
	     image_cache_(image_cache) {
	}
	virtual ~FontHandler1() {
	}

	// This will render the 'text' with a width restriction of 'w'. If 'w' == 0, no restriction is
	// applied.
	std::shared_ptr<const UI::RenderedText> render(const std::string& text,
	                                               uint16_t w = 0) override {
		const std::string hash = boost::lexical_cast<std::string>(w) + text;
		std::shared_ptr<const RenderedText> rendered_text = render_cache_->get(hash);
		if (rendered_text.get() == nullptr) {
			rendered_text = render_cache_->insert(hash, std::move(rt_renderer_->render(text, w)));
		}
		return rendered_text;
	}

	UI::FontSet const* fontset() const override {
		return fontset_;
	}

	void reinitialize_fontset(const std::string& locale) override {
		fontset_ = fontsets_.get_fontset(locale);
		texture_cache_->flush();
		render_cache_->flush();
		rt_renderer_.reset(new RT::Renderer(image_cache_, texture_cache_.get(), fontsets_));
	}

private:
	std::unique_ptr<TextureCache> texture_cache_;
	std::unique_ptr<RenderCache> render_cache_;
	UI::FontSets fontsets_;       // All fontsets
	UI::FontSet const* fontset_;  // The currently active FontSet
	std::unique_ptr<RT::Renderer> rt_renderer_;
	ImageCache* const image_cache_;  // not owned
};

IFontHandler1* create_fonthandler(ImageCache* image_cache, const std::string& locale) {
	return new FontHandler1(image_cache, locale);
}

IFontHandler1* g_fh1 = nullptr;

}  // namespace UI
