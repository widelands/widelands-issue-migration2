/*
 * Copyright (C) 2002-2021 by the Widelands Development Team
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

#include "wui/attack_box.h"

#include <memory>

#include <SDL_mouse.h>

#include "base/macros.h"
#include "base/multithreading.h"
#include "graphic/style_manager.h"
#include "graphic/text_layout.h"
#include "logic/map_objects/tribes/militarysite.h"
#include "logic/map_objects/tribes/soldier.h"
#include "map_io/map_object_loader.h"
#include "map_io/map_object_saver.h"
#include "wui/game_debug_ui.h"
#include "wui/interactive_player.h"

constexpr Duration kUpdateTimeInGametimeMs = Duration(500);  //  half a second, gametime
constexpr int kSpacing = 8;

AttackBox::AttackBox(InteractivePlayer& parent,
                     UI::UniqueWindow::Registry& reg,
                     const Widelands::Coords& target,
                     bool fastclick)
     : UI::UniqueWindow(&parent, UI::WindowStyle::kWui, "attack", &reg, 0, 0, _("Attack")),
     iplayer_(parent),
     map_(iplayer_.player().egbase().map()),
     node_coordinates_(target),
     lastupdate_(0) {
	init(fastclick);
}

std::vector<Widelands::Soldier*> AttackBox::get_max_attackers() {
	MutexLock m(MutexLock::ID::kObjects);

	if (upcast(Widelands::Building, building, map_.get_immovable(node_coordinates_))) {
		for (Widelands::Coords& coords : building->get_positions(iplayer_.egbase())) {
			if (iplayer_.player().is_seeing(map_.get_index(coords, map_.get_width()))) {
				std::vector<Widelands::Soldier*> v;
				// TODO(Nordfriese): This method decides by itself which soldier remains in the
				// building. This soldier will not show up in the result vector. Perhaps we should show
				// all available soldiers, grouped by building, so the player can choose between all
				// soldiers knowing that at least one of each group will have to stay at home. However,
				// this could clutter up the screen a lot. Especially if you have many small buildings.
				iplayer_.get_player()->find_attack_soldiers(building->base_flag(), &v);
				return v;
			}
		}
		// Player can't see any part of the building, so it can't be attacked
		// This is the same check as done later on in send_player_enemyflagaction()
	}
	return std::vector<Widelands::Soldier*>();
}

std::unique_ptr<UI::HorizontalSlider> AttackBox::add_slider(UI::Box& parent,
                                                            uint32_t width,
                                                            uint32_t height,
                                                            uint32_t min,
                                                            uint32_t max,
                                                            uint32_t initial,
                                                            char const* hint) {
	std::unique_ptr<UI::HorizontalSlider> result(new UI::HorizontalSlider(
	   &parent, 0, 0, width, height, min, max, initial, UI::SliderStyle::kWuiDark, hint));
	parent.add(result.get());
	return result;
}

UI::Textarea& AttackBox::add_text(UI::Box& parent,
                                  const std::string& str,
                                  UI::Align alignment,
                                  const UI::FontStyle style) {
	UI::Textarea& result =
	   *new UI::Textarea(&parent, UI::PanelStyle::kWui, style, str, UI::Align::kLeft);
	parent.add(&result, UI::Box::Resizing::kAlign, alignment);
	return result;
}

template <typename T>
std::unique_ptr<UI::Button> add_button(AttackBox* a,
                                                  UI::Box& parent,
                                                  const std::string& name,
                                                  const T& text_or_image,
                                                  void (AttackBox::*fn)(),
                                                  const std::string& tooltip_text) {
	std::unique_ptr<UI::Button> button(new UI::Button(
	   &parent, name, 8, 8, 34, 34, UI::ButtonStyle::kWuiPrimary, text_or_image, tooltip_text));
	button->sigclicked.connect([a, fn]() { (a->*fn)(); });
	parent.add(button.get());
	return button;
}

/*
 * Update available soldiers
 */
void AttackBox::think() {
	if (!iplayer_.player().is_seeing(iplayer_.egbase().map().get_index(node_coordinates_)) &&
	    !iplayer_.player().see_all()) {
		die();
	}

	if ((iplayer_.egbase().get_gametime() - lastupdate_) > kUpdateTimeInGametimeMs) {
		update_attack(false);
	}

	UI::Window::think();
}

static inline std::string slider_heading(uint32_t num_attackers) {
	/** TRANSLATORS: Number of soldiers that should attack. Used in Attack box. */
	return (boost::format(ngettext("%u soldier", "%u soldiers", num_attackers)) % num_attackers)
	   .str();
}

void AttackBox::update_attack(bool action_on_panel) {
	MutexLock m(MutexLock::ID::kObjects);

	lastupdate_ = iplayer_.egbase().get_gametime();

	assert(soldiers_slider_.get());
	assert(soldiers_text_.get());
	assert(less_soldiers_.get());
	assert(more_soldiers_.get());
	assert(attacking_soldiers_.get());
	assert(remaining_soldiers_.get());

	std::vector<Widelands::Soldier*> all_attackers = get_max_attackers();
	const int max_attackers = all_attackers.size();

	// Update number of available soldiers
	if (soldiers_slider_->get_max_value() != max_attackers) {
		soldiers_slider_->set_max_value(max_attackers);
	}

	// Add new soldiers and remove missing soldiers to/from the list
	for (const auto& s : all_attackers) {
		if (!attacking_soldiers_->contains(s) && !remaining_soldiers_->contains(s)) {
			remaining_soldiers_->add(s);
		}
	}
	for (const auto& s : remaining_soldiers_->get_soldiers()) {
		if (std::find(all_attackers.begin(), all_attackers.end(), s) == all_attackers.end()) {
			remaining_soldiers_->remove(s);
		}
	}
	for (const auto& s : attacking_soldiers_->get_soldiers()) {
		if (std::find(all_attackers.begin(), all_attackers.end(), s) == all_attackers.end()) {
			attacking_soldiers_->remove(s);
		}
	}

	if (action_on_panel) {
		// The player clicked on soldiers in the list – update slider
		soldiers_slider_->set_value(attacking_soldiers_->count_soldiers());
	} else {
		// The slider was moved or we were called from think() – shift lacking/extra soldiers between
		// the lists
		const int32_t lacking = soldiers_slider_->get_value() - attacking_soldiers_->count_soldiers();
		if (lacking > 0) {
			for (int32_t i = 0; i < lacking; ++i) {
				const Widelands::Soldier* s = remaining_soldiers_->get_soldier();
				remaining_soldiers_->remove(s);
				attacking_soldiers_->add(s);
			}
		} else if (lacking < 0) {
			for (int32_t i = 0; i > lacking; --i) {
				const Widelands::Soldier* s = attacking_soldiers_->get_soldier();
				attacking_soldiers_->remove(s);
				remaining_soldiers_->add(s);
			}
		}
	}

	// Update slider, buttons and texts
	soldiers_slider_->set_enabled(max_attackers > 0);
	more_soldiers_->set_enabled(max_attackers > soldiers_slider_->get_value());
	less_soldiers_->set_enabled(soldiers_slider_->get_value() > 0);

	soldiers_text_->set_text(slider_heading(soldiers_slider_->get_value()));

	more_soldiers_->set_title(std::to_string(max_attackers));
}

void AttackBox::init(const bool fastclick) {
	std::vector<Widelands::Soldier*> all_attackers = get_max_attackers();
	const size_t max_attackers = all_attackers.size();

	UI::Box& mainbox = *new UI::Box(this, UI::PanelStyle::kWui, 0, 0, UI::Box::Vertical);
	set_center_panel(&mainbox);

	UI::Box& linebox = *new UI::Box(&mainbox, UI::PanelStyle::kWui, 0, 0, UI::Box::Horizontal);
	mainbox.add(&linebox, UI::Box::Resizing::kFullSize);

	less_soldiers_ = add_button(this, linebox, "less", "0", &AttackBox::send_less_soldiers,
	                            _("Send less soldiers. Hold down Ctrl to send no soldiers"));

	UI::Box& columnbox = *new UI::Box(&linebox, UI::PanelStyle::kWui, 0, 0, UI::Box::Vertical);
	linebox.add(&columnbox);

	soldiers_text_.reset(&add_text(columnbox, slider_heading(max_attackers > 0 ? 1 : 0),
	                               UI::Align::kCenter, UI::FontStyle::kWuiAttackBoxSliderLabel));

	soldiers_slider_ = add_slider(
	   columnbox, 210, 17, 0, max_attackers, max_attackers > 0 ? 1 : 0, _("Number of soldiers"));
	soldiers_slider_->changed.connect([this]() { update_attack(false); });

	more_soldiers_ =
	   add_button(this, linebox, "more", std::to_string(max_attackers), &AttackBox::send_more_soldiers,
	              _("Send more soldiers. Hold down Ctrl to send as many soldiers as possible"));
	linebox.add_space(kSpacing);

	attack_button_.reset(new UI::Button(
	   &linebox, "attack", 8, 8, 34, 34, UI::ButtonStyle::kWuiPrimary,
	   g_image_cache->get("images/wui/buildings/menu_attack.png"), _("Start attack")));
	linebox.add(attack_button_.get());
	attack_button_->sigclicked.connect([this]() { act_attack(); });
	set_fastclick_panel(attack_button_.get());

	attacking_soldiers_.reset(new ListOfSoldiers(&mainbox, this, 0, 0, 30, 30));
	remaining_soldiers_.reset(new ListOfSoldiers(&mainbox, this, 0, 0, 30, 30));
	attacking_soldiers_->set_complement(remaining_soldiers_.get());
	remaining_soldiers_->set_complement(attacking_soldiers_.get());
	for (const auto& s : all_attackers) {
		remaining_soldiers_->add(s);
	}

	boost::format tooltip_format("<p>%s%s%s</p>");
	{
		UI::Textarea& txt =
		   add_text(mainbox, _("Attackers:"), UI::Align::kLeft, UI::FontStyle::kWuiLabel);
		// Needed so we can get tooltips
		txt.set_handle_mouse(true);
		txt.set_tooltip(
		   (tooltip_format %
		    g_style_manager->font_style(UI::FontStyle::kWuiTooltipHeader)
		       .as_font_tag(_("Click on a soldier to remove him from the list of attackers")) %
		    as_listitem(
		       _("Hold down Ctrl to remove all soldiers from the list"), UI::FontStyle::kWuiTooltip) %
		    as_listitem(_("Hold down Shift to remove all soldiers up to the one you’re pointing at"),
		                UI::FontStyle::kWuiTooltip))
		      .str());
		mainbox.add(attacking_soldiers_.get(), UI::Box::Resizing::kFullSize);
	}

	{
		UI::Textarea& txt =
		   add_text(mainbox, _("Not attacking:"), UI::Align::kLeft, UI::FontStyle::kWuiLabel);
		txt.set_handle_mouse(true);
		txt.set_tooltip(
		   (tooltip_format %
		    g_style_manager->font_style(UI::FontStyle::kWuiTooltipHeader)
		       .as_font_tag(_("Click on a soldier to add him to the list of attackers")) %
		    as_listitem(
		       _("Hold down Ctrl to add all soldiers to the list"), UI::FontStyle::kWuiTooltip) %
		    as_listitem(_("Hold down Shift to add all soldiers up to the one you’re pointing at"),
		                UI::FontStyle::kWuiTooltip))
		      .str());
		mainbox.add(remaining_soldiers_.get(), UI::Box::Resizing::kFullSize);
	}

	UI::Box& bottombox = *new UI::Box(&mainbox, UI::PanelStyle::kWui, 0, 0, UI::Box::Horizontal);
	mainbox.add(&bottombox, UI::Box::Resizing::kFullSize);

	const Widelands::BaseImmovable* i = map_.get_immovable(node_coordinates_);
	if (i && i->descr().type() == Widelands::MapObjectType::MILITARYSITE) {
		do_not_conquer_.reset(
		   new UI::Checkbox(&bottombox, UI::PanelStyle::kWui, Vector2i(0, 0), _("Destroy target"),
		                    _("Destroy the target building instead of conquering it")));
		do_not_conquer_->set_state(
		   !dynamic_cast<const Widelands::MilitarySite&>(*i).attack_target()->get_allow_conquer(
		      iplayer_.player_number()));
		bottombox.add(do_not_conquer_.get(), UI::Box::Resizing::kAlign, UI::Align::kBottom);
	}
	bottombox.add_inf_space();

	if (iplayer_.get_display_flag(InteractiveBase::dfDebug)) {
		add_button(this, bottombox, "debug", g_image_cache->get("images/wui/fieldaction/menu_debug.png"), &AttackBox::act_debug, _("Show Debug Window")).release();
		bottombox.add_space(kSpacing);
	}
	add_button(this, bottombox, "goto", g_image_cache->get("images/wui/menus/goto.png"), &AttackBox::act_goto, _("Center view on this")).release();

	soldiers_slider_->set_enabled(max_attackers > 0);
	more_soldiers_->set_enabled(max_attackers > 0);
	// Update the list of soldiers now to avoid a flickering window in the next tick
	update_attack(false);

	center_to_parent();
	if (fastclick) {
		warp_mouse_to_fastclick_panel();
	}
	initialization_complete();
}

/** The attack button was pressed. */
void AttackBox::act_attack() {
	MutexLock m(MutexLock::ID::kObjects);
	if (upcast(Widelands::Building, building, iplayer_.egbase().map().get_immovable(node_coordinates_))) {
		iplayer_.game().send_player_enemyflagaction(building->base_flag(), iplayer_.player_number(),
		                                  soldiers(), get_allow_conquer());
	}
	die();
}

/** Center the player's view on the building. Callback function for the corresponding button. */
void AttackBox::act_goto() {
	iplayer_.map_view()->scroll_to_field(node_coordinates_, MapView::Transition::Smooth);
}

/** Callback for debug window. */
void AttackBox::act_debug() {
	show_field_debug(iplayer_, iplayer_.egbase().map().get_fcoords(node_coordinates_));
}

void AttackBox::send_less_soldiers() {
	assert(soldiers_slider_.get());
	soldiers_slider_->set_value((SDL_GetModState() & KMOD_CTRL) ? 0 :
                                                                 soldiers_slider_->get_value() - 1);
}

void AttackBox::send_more_soldiers() {
	soldiers_slider_->set_value((SDL_GetModState() & KMOD_CTRL) ? soldiers_slider_->get_max_value() :
                                                                 soldiers_slider_->get_value() + 1);
}

size_t AttackBox::count_soldiers() const {
	return attacking_soldiers_->count_soldiers();
}

std::vector<Widelands::Serial> AttackBox::soldiers() const {
	std::vector<Widelands::Serial> result;
	for (const auto& s : attacking_soldiers_->get_soldiers()) {
		result.push_back(s->serial());
	}
	return result;
}

constexpr int kSoldierIconWidth = 32;
constexpr int kSoldierIconHeight = 30;

AttackBox::ListOfSoldiers::ListOfSoldiers(UI::Panel* const parent,
                                          AttackBox* parent_box,
                                          int32_t const x,
                                          int32_t const y,
                                          int const w,
                                          int const h,
                                          bool restrict_rows)
   : UI::Panel(parent, UI::PanelStyle::kWui, x, y, w, h),
     restricted_row_number_(restrict_rows),
     attack_box_(parent_box) {
	update_desired_size();
}

bool AttackBox::ListOfSoldiers::handle_mousepress(uint8_t btn, int32_t x, int32_t y) {
	if (btn != SDL_BUTTON_LEFT || !other_) {
		return UI::Panel::handle_mousepress(btn, x, y);
	}
	if (SDL_GetModState() & KMOD_CTRL) {
		for (const auto& s : get_soldiers()) {
			remove(s);
			other_->add(s);
		}
	} else {
		const Widelands::Soldier* soldier = soldier_at(x, y);
		if (!soldier) {
			return UI::Panel::handle_mousepress(btn, x, y);
		}
		if (SDL_GetModState() & KMOD_SHIFT) {
			for (const auto& s : get_soldiers()) {
				remove(s);
				other_->add(s);
				if (s == soldier) {
					break;
				}
			}
		} else {
			remove(soldier);
			other_->add(soldier);
		}
	}
	attack_box_->update_attack(true);
	return true;
}

void AttackBox::ListOfSoldiers::handle_mousein(bool) {
	set_tooltip(std::string());
}

bool AttackBox::ListOfSoldiers::handle_mousemove(uint8_t, int32_t x, int32_t y, int32_t, int32_t) {
	if (const Widelands::Soldier* soldier = soldier_at(x, y)) {
		set_tooltip((boost::format(_("HP: %1$u/%2$u  AT: %3$u/%4$u  DE: %5$u/%6$u  EV: %7$u/%8$u")) %
		             soldier->get_health_level() % soldier->descr().get_max_health_level() %
		             soldier->get_attack_level() % soldier->descr().get_max_attack_level() %
		             soldier->get_defense_level() % soldier->descr().get_max_defense_level() %
		             soldier->get_evade_level() % soldier->descr().get_max_evade_level())
		               .str());
	} else {
		set_tooltip(std::string());
	}
	return true;
}

// whole window
bool AttackBox::handle_mousewheel(int32_t x, int32_t y, uint16_t modstate) {
	return soldiers_slider_->handle_mousewheel(x, y, modstate);
}

Widelands::Extent AttackBox::ListOfSoldiers::size() const {
	const size_t nr_soldiers = count_soldiers();
	uint32_t rows = nr_soldiers / current_size_;
	if (nr_soldiers == 0 || rows * current_size_ < nr_soldiers) {
		++rows;
	}
	if (restricted_row_number_) {
		return Widelands::Extent(rows, current_size_);
	}
	return Widelands::Extent(current_size_, rows);
}

void AttackBox::ListOfSoldiers::update_desired_size() {
	current_size_ = std::max(
	   1, restricted_row_number_ ? get_h() / kSoldierIconHeight : get_w() / kSoldierIconWidth);
	const Widelands::Extent e = size();
	set_desired_size(e.w * kSoldierIconWidth, e.h * kSoldierIconHeight);
}

const Widelands::Soldier* AttackBox::ListOfSoldiers::soldier_at(int32_t x, int32_t y) const {
	if (x < 0 || y < 0 || soldiers_.empty()) {
		return nullptr;
	}
	const int32_t col = x / kSoldierIconWidth;
	const int32_t row = y / kSoldierIconHeight;
	assert(col >= 0);
	assert(row >= 0);
	if ((restricted_row_number_ ? row : col) >= current_size_) {
		return nullptr;
	}
	const int index = restricted_row_number_ ? current_size_ * col + row : current_size_ * row + col;
	assert(index >= 0);
	return static_cast<unsigned int>(index) < soldiers_.size() ? soldiers_[index] : nullptr;
}

void AttackBox::ListOfSoldiers::add(const Widelands::Soldier* s) {
	soldiers_.push_back(s);
	update_desired_size();
}

void AttackBox::ListOfSoldiers::remove(const Widelands::Soldier* s) {
	const auto it = std::find(soldiers_.begin(), soldiers_.end(), s);
	assert(it != soldiers_.end());
	soldiers_.erase(it);
	update_desired_size();
}

void AttackBox::ListOfSoldiers::draw(RenderTarget& dst) {
	const size_t nr_soldiers = soldiers_.size();
	int32_t column = 0;
	int32_t row = 0;
	for (uint32_t i = 0; i < nr_soldiers; ++i) {
		Vector2i location(column * kSoldierIconWidth, row * kSoldierIconHeight);
		soldiers_[i]->draw_info_icon(
		   location, 1.0f, Widelands::Soldier::InfoMode::kInBuilding, InfoToDraw::kSoldierLevels, &dst);
		if (restricted_row_number_) {
			++row;
			if (row >= current_size_) {
				row = 0;
				++column;
			}
		} else {
			++column;
			if (column >= current_size_) {
				column = 0;
				++row;
			}
		}
	}
}

constexpr uint16_t kCurrentPacketVersion = 1;
UI::Window& AttackBox::load(FileRead& fr, InteractiveBase& ib, Widelands::MapObjectLoader& mol) {
	try {
		const uint16_t packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersion) {
			const int32_t x = fr.signed_32();
			const int32_t y = fr.signed_32();
			AttackBox* a = dynamic_cast<AttackBox*>(dynamic_cast<InteractivePlayer&>(ib).show_attack_box(Widelands::Coords(x, y), false));
			assert(a != nullptr);

			const uint8_t destroy = fr.unsigned_8();
			if (a->do_not_conquer_ != nullptr) {
				a->do_not_conquer_->set_state(destroy == 0);
			}

			for (const Widelands::Soldier* s : a->attacking_soldiers_->get_soldiers()) {
				a->attacking_soldiers_->remove(s);
				a->remaining_soldiers_->add(s);
			}
			for (size_t i = fr.unsigned_32(); i; --i) {
				const Widelands::Soldier* s = &mol.get<Widelands::Soldier>(fr.unsigned_32());
				a->remaining_soldiers_->remove(s);
				a->attacking_soldiers_->add(s);
			}

			return *a;
		} else {
			throw Widelands::UnhandledVersionError(
			   "Attack Box", packet_version, kCurrentPacketVersion);
		}
	} catch (const WException& e) {
		throw Widelands::GameDataError("attack box: %s", e.what());
	}
}
void AttackBox::save(FileWrite& fw, Widelands::MapObjectSaver& mos) const {
	fw.unsigned_16(kCurrentPacketVersion);
	fw.signed_32(node_coordinates_.x);
	fw.signed_32(node_coordinates_.y);

	fw.unsigned_8(do_not_conquer_ && !do_not_conquer_->get_state() ? 1 : 0);

	fw.unsigned_32(attacking_soldiers_->get_soldiers().size());
	for (const Widelands::Soldier* s : attacking_soldiers_->get_soldiers()) {
		fw.unsigned_32(mos.get_object_file_index(*s));
	}
}
