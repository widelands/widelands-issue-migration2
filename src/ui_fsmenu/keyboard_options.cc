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

#include "ui_fsmenu/keyboard_options.h"

#include <boost/format.hpp>

#include "base/i18n.h"
#include "logic/map_objects/tribes/tribe_descr.h"
#include "ui_basic/dropdown.h"
#include "ui_basic/messagebox.h"
#include "ui_basic/multilinetextarea.h"
#include "wlapplication_options.h"

namespace FsMenu {

constexpr int16_t kPadding = 4;

struct ShortcutChooser : public UI::Window {
	ShortcutChooser(UI::Panel& parent, const KeyboardShortcut c, Widelands::Game* game_for_fastplace)
	   : UI::Window(
	        &parent, UI::WindowStyle::kFsMenu, "choose_shortcut", 0, 0, 300, 200, to_string(c)),
	     key(get_shortcut(c)) {
		UI::Box* const box =
		   new UI::Box(this, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Vertical, 0, 0, kPadding);

		UI::Button* const reset = new UI::Button(
		   box, "reset", 0, 0, 0, 0, UI::ButtonStyle::kFsMenuSecondary, _("Reset to default"));
		reset->sigclicked.connect([this, c]() {
			key = get_default_shortcut(c);
			end_modal<UI::Panel::Returncodes>(UI::Panel::Returncodes::kOk);
		});

		UI::Button* const cancel =
		   new UI::Button(box, "cancel", 0, 0, 0, 0, UI::ButtonStyle::kFsMenuSecondary, _("Cancel"));
		cancel->sigclicked.connect(
		   [this]() { end_modal<UI::Panel::Returncodes>(UI::Panel::Returncodes::kBack); });

		UI::Button* const ok =
		   new UI::Button(box, "ok", 0, 0, 0, 0, UI::ButtonStyle::kFsMenuPrimary, _("OK"));
		ok->sigclicked.connect(
		   [this]() { end_modal<UI::Panel::Returncodes>(UI::Panel::Returncodes::kOk); });

		UI::MultilineTextarea* const txt = new UI::MultilineTextarea(
		   box, 0, 0, 200, 100, UI::PanelStyle::kFsMenu,
		   _("Press the new shortcut or close this window to cancel."), UI::Align::kCenter);

		box->add(txt, UI::Box::Resizing::kExpandBoth);

		if (game_for_fastplace) {
			fastplace = get_fastplace_shortcut(c);
			UI::Dropdown<std::string>* dd = new UI::Dropdown<std::string>(box, "choose_fastplace", 0, 0, 100, 8,
					ok->get_h(), "", UI::DropdownType::kTextual, UI::PanelStyle::kFsMenu, UI::ButtonStyle::kFsMenuMenu);
			dd->add(_("(unused)"), "", nullptr, fastplace.empty(), _("Do not use this fastplace slot"));

			std::map<std::pair<Widelands::DescriptionIndex, std::string>, const Widelands::BuildingDescr*> all_building_sorted;
			for (Widelands::DescriptionIndex di = 0; di < game_for_fastplace->descriptions().nr_buildings(); ++di) {
				const Widelands::BuildingDescr* bld = game_for_fastplace->descriptions().get_building_descr(di);
				if (bld->is_buildable()) {
					all_building_sorted[std::make_pair(game_for_fastplace->descriptions().safe_tribe_index(bld->get_owning_tribe()), bld->descname())] = bld;
				}
			}
			for (const auto& pair : all_building_sorted) {
				dd->add((boost::format(
					/** TRANSLATORS: [Tribe Name] Building Name */
					_("[%1$s] %2$s"))
						% game_for_fastplace->descriptions().get_tribe_descr(pair.first.first)->descname()
						% pair.second->descname()).str(),
					pair.second->name(), pair.second->icon(), fastplace == pair.second->name());
			}

			if (!dd->has_selection()) {
				// The assigned building is defined by a currently disabled add-on.
				dd->add(fastplace, fastplace, nullptr, true);
			}

			dd->selected.connect([this, dd]() { fastplace = dd->get_selected(); });
			box->add(dd, UI::Box::Resizing::kFullSize);
		}

		box->add(reset, UI::Box::Resizing::kFullSize);
		box->add(cancel, UI::Box::Resizing::kFullSize);
		box->add(ok, UI::Box::Resizing::kFullSize);
		set_center_panel(box);
		center_to_parent();
	}

	SDL_Keysym key;
	std::string fastplace;

	bool handle_key(const bool down, const SDL_Keysym code) override {
		if (!down) {
			return false;
		}

		switch (code.sym) {

		// Ignore modifiers
		case SDLK_RCTRL:
		case SDLK_RSHIFT:
		case SDLK_RALT:
		case SDLK_RGUI:
		case SDLK_LCTRL:
		case SDLK_LSHIFT:
		case SDLK_LALT:
		case SDLK_LGUI:
			return false;

		// Also ignore reserved system keys
		case SDLK_RETURN:
		case SDLK_KP_ENTER:
		case SDLK_SPACE:
		case SDLK_ESCAPE:
		case SDLK_UP:
		case SDLK_DOWN:
		case SDLK_LEFT:
		case SDLK_RIGHT:
		case SDLK_BACKSPACE:
		case SDLK_TAB:
			return false;

		default:
			key = code;
			end_modal<UI::Panel::Returncodes>(UI::Panel::Returncodes::kOk);
			return true;
		}
	}
};

KeyboardOptions::KeyboardOptions(Panel& parent)
   : UI::Window(parent.get_parent(),
                UI::WindowStyle::kFsMenu,
                "keyboard_options",
                0,
                0,
                parent.get_parent()->get_w() * 3 / 4,
                parent.get_h() * 3 / 4,
                _("Edit Keyboard Shortcuts")),
     buttons_box_(this, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Horizontal, 0, 0, kPadding),
     tabs_(this, UI::TabPanelStyle::kFsMenu),
     reset_(&buttons_box_,
            "reset",
            0,
            0,
            get_w() / 3,
            28,
            UI::ButtonStyle::kFsMenuSecondary,
            _("Reset all"),
            _("Reset all to defaults")),
     ok_(&buttons_box_,
         "ok",
         0,
         0,
         reset_.get_w(),
         reset_.get_h(),
         UI::ButtonStyle::kFsMenuPrimary,
         _("OK")) {
	std::map<KeyboardShortcut, UI::Button*> all_keyboard_buttons;

	auto generate_title = [](const KeyboardShortcut key) {
		const std::string shortcut = shortcut_string_for(key, false);
		if (key >= KeyboardShortcut::kFastplace__Begin && key <= KeyboardShortcut::kFastplace__End) {
			return shortcut.empty() ? _("(unused)") : (boost::format(_("%1$s: %2$s")) % get_fastplace_shortcut(key) % shortcut).str();
		}
		return (boost::format(_("%1$s: %2$s")) % to_string(key) % shortcut).str();
	};

	auto add_key = [this, generate_title, &all_keyboard_buttons](
	                  UI::Box& box, const KeyboardShortcut key) {
		UI::Button* b = new UI::Button(&box, std::to_string(static_cast<int>(key)), 0, 0, 0, 0,
		                               UI::ButtonStyle::kFsMenuMenu, generate_title(key));
		all_keyboard_buttons.emplace(std::make_pair(key, b));
		box.add(b, UI::Box::Resizing::kFullSize);
		box.add_space(kPadding);
		b->sigclicked.connect([this, b, key, generate_title]() {
			const bool fastplace = (key >= KeyboardShortcut::kFastplace__Begin && key <= KeyboardShortcut::kFastplace__End);
			ShortcutChooser c(*get_parent(), key, fastplace ? game_.get() : nullptr);
			if (c.run<UI::Panel::Returncodes>() == UI::Panel::Returncodes::kOk) {
				KeyboardShortcut conflict;
				if (set_shortcut(key, c.key, &conflict)) {
					if (fastplace) {
						set_fastplace_shortcut(key, c.fastplace);
					}
					b->set_title(generate_title(key));
				} else {
					UI::WLMessageBox warning(
					   get_parent(), UI::WindowStyle::kFsMenu, _("Keyboard Shortcut Conflict"),
					   (boost::format(_("The shortcut you selected (‘%1$s’) is already in use for the "
					                    "following action: ‘%2$s’. Please select a different shortcut "
					                    "or change the conflicting shortcut first.")) %
					    shortcut_string_for(c.key, false) % to_string(conflict))
					      .str(),
					   UI::WLMessageBox::MBoxType::kOk, UI::Align::kCenter, true);
					warning.run<UI::Panel::Returncodes>();
				}
			}
		});
	};

	auto create_tab = [this, add_key](const std::string& title,
	                                  const KeyboardShortcut shortcut_start,
	                                  const KeyboardShortcut shortcut_end) {
		const uint16_t s1 = static_cast<uint16_t>(shortcut_start);
		const uint16_t s2 = static_cast<uint16_t>(shortcut_end);
		assert(s1 < s2);
		UI::Box* b =
		   new UI::Box(&tabs_, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Vertical, 0, 0, kPadding);
		b->set_force_scrolling(true);
		for (uint16_t k = s1; k <= s2; ++k) {
			add_key(*b, static_cast<KeyboardShortcut>(k));
		}
		tabs_.add(title, title, b, "");
		boxes_.push_back(b);
	};
	create_tab(_("General"), KeyboardShortcut::kCommon__Begin, KeyboardShortcut::kCommon__End);
	create_tab(_("Main Menu"), KeyboardShortcut::kMainMenu__Begin, KeyboardShortcut::kMainMenu__End);
	create_tab(_("Editor"), KeyboardShortcut::kEditor__Begin, KeyboardShortcut::kEditor__End);
	create_tab(_("Game"), KeyboardShortcut::kInGame__Begin, KeyboardShortcut::kInGame__End);
	create_tab(_("Fastplace"), KeyboardShortcut::kFastplace__Begin, KeyboardShortcut::kFastplace__End);

	buttons_box_.add_inf_space();
	buttons_box_.add(&reset_, UI::Box::Resizing::kAlign, UI::Align::kCenter);
	buttons_box_.add_inf_space();
	buttons_box_.add(&ok_, UI::Box::Resizing::kAlign, UI::Align::kCenter);
	buttons_box_.add_inf_space();

	tabs_.sigclicked.connect([this]() {
		if (tabs_.active() == 4 && game_.get() == nullptr) {
			game_.reset(new Widelands::Game());
			game_->create_loader_ui({}, false, "", "", this);
			game_->load_all_tribes();
			game_->postload_addons();
			game_->remove_loader_ui();
		}
	});
	reset_.sigclicked.connect([all_keyboard_buttons, generate_title]() {
		init_shortcuts(true);
		for (auto& pair : all_keyboard_buttons) {
			pair.second->set_title(generate_title(pair.first));
		}
	});
	ok_.sigclicked.connect([this]() { die(); });

	layout();
	center_to_parent();
}

bool KeyboardOptions::handle_key(bool down, SDL_Keysym code) {
	if (down && (code.sym == SDLK_KP_ENTER || code.sym == SDLK_RETURN)) {
		die();
		return true;
	}
	return UI::Window::handle_key(down, code);
}

void KeyboardOptions::layout() {
	if (!is_minimal()) {
		int w, h;
		buttons_box_.get_desired_size(&w, &h);
		buttons_box_.set_size(get_inner_w(), h);
		buttons_box_.set_pos(Vector2i(0, get_inner_h() - h));
		tabs_.set_size(get_inner_w(), get_inner_h() - h - kPadding);
		for (UI::Box* b : boxes_) {
			b->set_max_size(tabs_.get_inner_w(), tabs_.get_inner_h());
		}
	}
	UI::Window::layout();
}

}  // namespace FsMenu
