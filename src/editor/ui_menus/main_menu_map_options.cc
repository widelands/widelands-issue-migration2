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

#include "editor/ui_menus/main_menu_map_options.h"

#include "base/i18n.h"
#include "editor/editorinteractive.h"
#include "graphic/font_handler.h"
#include "graphic/graphic.h"
#include "graphic/text_layout.h"
#include "logic/map.h"
#include "logic/note_map_options.h"
#include "ui_basic/editbox.h"
#include "ui_basic/multilineeditbox.h"
#include "ui_basic/textarea.h"
#include "wlapplication_options.h"
#include "wui/map_tags.h"

inline EditorInteractive& MainMenuMapOptions::eia() {
	return dynamic_cast<EditorInteractive&>(*get_parent());
}

constexpr unsigned kSuggestedTeamsUnitSize = 24;

SuggestedTeamsEntry::SuggestedTeamsEntry(MainMenuMapOptions* mmmo, UI::Panel* parent, const Widelands::Map& map, unsigned w, Widelands::SuggestedTeamLineup t)
: UI::Panel(parent, 0, 0, w, kSuggestedTeamsUnitSize, _("Click player to remove")),
map_(map),
team_(t),
delete_(this, "delete", 0, 0, kSuggestedTeamsUnitSize, kSuggestedTeamsUnitSize,
	    UI::ButtonStyle::kWuiSecondary,
	    _("Delete"),
	    _("Delete this suggested team lineup")) {
	for (size_t index = 0; index < team_.size(); ++index) {
		dropdowns_.push_back(create_dropdown(index));
	}

	delete_.sigclicked.connect([this, mmmo]() {
		mmmo->delete_suggested_team(this);
	});

	update();
}

UI::Dropdown<Widelands::PlayerNumber>* SuggestedTeamsEntry::create_dropdown(size_t index) {
	UI::Dropdown<Widelands::PlayerNumber>* dd = new UI::Dropdown<Widelands::PlayerNumber>(this, std::to_string(index),
	         0,
	         index * kSuggestedTeamsUnitSize,
	         kSuggestedTeamsUnitSize,
	         8,
	         0,
	         _("+"),
	         UI::DropdownType::kPictorialMenu,
	         UI::PanelStyle::kWui,
	         UI::ButtonStyle::kWuiSecondary);
	for (size_t i = 0; i < map_.get_nrplayers(); ++i) {
		dd->add(map_.get_scenario_player_name(i + 1), i,
	         playercolor_image(i, "images/players/player_position_menu.png"));
	}
	dd->set_tooltip(_("Add a player to this team"));
	dd->selected.connect([this, dd]() {
		const Widelands::PlayerNumber player = dd->get_selected();
		// add this player to this team and remove him from all other teams in this lineup
		for (Widelands::SuggestedTeam& t : team_) {
			for (auto it = t.begin(); it != t.end(); ++it) {
				if (*it == player) {
					t.erase(it);
					break;
				}
			}
		}
		// determine our index (it may have changed since the creation!)
		int dd_index = -1;
		for (size_t i = 0; i < dropdowns_.size(); ++i) {
			if (dropdowns_[i] == dd) {
				dd_index = i;
				break;
			}
		}
		assert(dd_index >= 0);
		if (static_cast<unsigned>(dd_index) >= team_.size()) {
			assert(static_cast<unsigned>(dd_index) == team_.size());
			team_.push_back(Widelands::SuggestedTeam());
		}
		team_[dd_index].push_back(player);
		update();
	});
	return dd;
}

void SuggestedTeamsEntry::layout() {
	const uint16_t h = kSuggestedTeamsUnitSize * (team_.size() + 1);
	set_desired_size(get_w(), h);
	delete_.set_pos(Vector2i(get_w() / 2, h - kSuggestedTeamsUnitSize));
	delete_.set_size(get_w() / 2, kSuggestedTeamsUnitSize);

	size_t index = 0;
	for (auto& dd : dropdowns_) {
		dd->set_size(kSuggestedTeamsUnitSize, kSuggestedTeamsUnitSize);
		dd->set_pos(Vector2i(0, kSuggestedTeamsUnitSize * (index++)));
	}
}

// Delete empty teams, and append an empty team to the end if not present
void SuggestedTeamsEntry::update() {
	int nr_teams = team_.size();
	int nr_dd = dropdowns_.size();
	for (int i = 0; i < nr_teams;) {
		if (team_[i].empty()) {
			dropdowns_[i]->die();

			for (int j = i + 1; j < nr_dd; ++j) {
				dropdowns_[j - 1] = dropdowns_[j];
			}
			dropdowns_.resize(nr_dd - 1);
			for (int j = i + 1; j < nr_teams; ++j) {
				team_[j - 1] = team_[j];
			}
			team_.resize(nr_teams - 1);

			--nr_teams;
			--nr_dd;
		} else {
			++i;
		}
	}
	assert(nr_teams >= 0);
	assert(nr_dd >= 0);

	assert((nr_dd == nr_teams) ^ (nr_dd == nr_teams + 1));
	if (nr_dd <= nr_teams) {
		dropdowns_.push_back(create_dropdown(nr_teams));
	}

	layout();
}

bool SuggestedTeamsEntry::handle_mousepress(uint8_t btn, int32_t x, int32_t y) {
	const int row = y / kSuggestedTeamsUnitSize;
	const int col = x / kSuggestedTeamsUnitSize;
	if (row >= 0 && static_cast<unsigned>(row) < team_.size()) {
		if (col > 0 && static_cast<unsigned>(col) <= team_[row].size()) {
			team_[row].erase(team_[row].begin() + (col - 1));
			update();
			return true;
		}
	}

	return UI::Panel::handle_mousepress(btn, x, y);
}

void SuggestedTeamsEntry::draw(RenderTarget& r) {
	UI::Panel::draw(r);

	for (size_t row = 0; row < team_.size(); ++row) {
		for (size_t col = 0; col < team_[row].size(); ++col) {
			r.blit(Vector2i((col + 1) * kSuggestedTeamsUnitSize, row * kSuggestedTeamsUnitSize),
					playercolor_image(team_[row][col], "images/players/player_position_menu.png"));
		}
	}
}

constexpr uint16_t kMaxRecommendedWaterwayLengthLimit = 20;

/**
 * Create all the buttons etc...
 */
MainMenuMapOptions::MainMenuMapOptions(EditorInteractive& parent, Registry& registry)
   : UI::UniqueWindow(
        &parent, "map_options", &registry, 350, parent.get_inner_h() - 80, _("Map Options")),
     padding_(4),
     indent_(10),
     labelh_(text_height(UI::FontStyle::kLabel) + 4),
     checkbox_space_(25),
     butw_((get_inner_w() - 3 * padding_) / 2),
     max_w_(get_inner_w() - 2 * padding_),
     ok_(this,
         "ok",
         UI::g_fh->fontset()->is_rtl() ? padding_ : butw_ + 2 * padding_,
         get_inner_h() - padding_ - labelh_,
         butw_,
         labelh_,
         UI::ButtonStyle::kWuiPrimary,
         _("OK")),
     cancel_(this,
             "cancel",
             UI::g_fh->fontset()->is_rtl() ? butw_ + 2 * padding_ : padding_,
             get_inner_h() - padding_ - labelh_,
             butw_,
             labelh_,
             UI::ButtonStyle::kWuiSecondary,
             _("Cancel")),
     tab_box_(this, padding_, padding_, UI::Box::Vertical, max_w_, get_inner_h(), 0),
     tabs_(&tab_box_, UI::TabPanelStyle::kWuiLight),

     main_box_(&tabs_, padding_, padding_, UI::Box::Vertical, max_w_, get_inner_h(), 0),
     tags_box_(&tabs_, padding_, padding_, UI::Box::Vertical, max_w_, get_inner_h(), 0),
     teams_box_(&tabs_, padding_, padding_, UI::Box::Vertical, max_w_, get_inner_h(), 0),
     inner_teams_box_(&teams_box_, padding_, padding_, UI::Box::Vertical, max_w_, get_inner_h(), 0),

     name_(&main_box_, 0, 0, max_w_, UI::PanelStyle::kWui),
     author_(&main_box_, 0, 0, max_w_, UI::PanelStyle::kWui),
     size_(&main_box_, 0, 0, max_w_ - indent_, labelh_, ""),
     balancing_dropdown_(&tags_box_,
                         "dropdown_balancing",
                         0,
                         0,
                         200,
                         50,
                         24,
                         "",
                         UI::DropdownType::kTextual,
                         UI::PanelStyle::kWui,
                         UI::ButtonStyle::kWuiSecondary),
     new_suggested_team_(&teams_box_, "new_suggested_team", 0, 0, max_w_, kSuggestedTeamsUnitSize,
	    UI::ButtonStyle::kWuiSecondary,
	    _("Add lineup"),
	    _("Add another suggested team lineup")),
     registry_(registry) {

	tab_box_.set_size(max_w_, get_inner_h() - labelh_ - 2 * padding_);
	tabs_.set_size(max_w_, tab_box_.get_inner_h());
	main_box_.set_size(max_w_, tabs_.get_inner_h() - 35);
	tags_box_.set_size(max_w_, main_box_.get_h());
	teams_box_.set_size(max_w_, main_box_.get_h());

	// Calculate the overall remaining space for MultilineEditboxes.
	uint32_t remaining_space = main_box_.get_inner_h() - 7 * labelh_ - 5 * indent_;

	// We need less space for the hint and the description, but it should at least have 1 line
	// height.
	hint_ = new UI::MultilineEditbox(
	   &main_box_, 0, 0, max_w_, std::max(labelh_, remaining_space * 1 / 3), UI::PanelStyle::kWui);
	descr_ = new UI::MultilineEditbox(
	   &main_box_, 0, 0, max_w_, remaining_space - hint_->get_h(), UI::PanelStyle::kWui);

	main_box_.add(new UI::Textarea(&main_box_, 0, 0, max_w_, labelh_, _("Map name:")));
	main_box_.add(&name_);
	main_box_.add_space(indent_);

	main_box_.add(new UI::Textarea(&main_box_, 0, 0, max_w_, labelh_, _("Authors:")));
	main_box_.add(&author_);
	main_box_.add_space(indent_);

	main_box_.add(new UI::Textarea(&main_box_, 0, 0, max_w_, labelh_, _("Description:")));
	main_box_.add(descr_);
	main_box_.add_space(indent_);

	main_box_.add(new UI::Textarea(&main_box_, 0, 0, max_w_, labelh_, _("Hint (optional):")));
	main_box_.add(hint_);
	main_box_.add_space(indent_);

	main_box_.add(&size_);
	main_box_.add_space(indent_);

	tags_box_.add(new UI::Textarea(&tags_box_, 0, 0, max_w_, labelh_, _("Tags:")));
	add_tag_checkbox(&tags_box_, "ffa", localize_tag("ffa"));
	add_tag_checkbox(&tags_box_, "1v1", localize_tag("1v1"));
	add_tag_checkbox(&tags_box_, "2teams", localize_tag("2teams"));
	add_tag_checkbox(&tags_box_, "3teams", localize_tag("3teams"));
	add_tag_checkbox(&tags_box_, "4teams", localize_tag("4teams"));

	balancing_dropdown_.set_autoexpand_display_button();
	balancing_dropdown_.add(localize_tag("balanced"), "balanced");
	balancing_dropdown_.add(localize_tag("unbalanced"), "unbalanced");
	tags_box_.add(&balancing_dropdown_);

	tags_box_.add_space(labelh_);

	tags_box_.add(new UI::Textarea(&tags_box_, 0, 0, max_w_, labelh_, _("Waterway length limit:")));
	UI::Box* ww_box = new UI::Box(&tags_box_, 0, 0, UI::Box::Horizontal, max_w_);
	waterway_length_warning_ = new UI::Icon(ww_box, g_gr->images().get("images/ui_basic/stop.png"));
	waterway_length_warning_->set_handle_mouse(true);
	waterway_length_box_ =
	   new UI::SpinBox(ww_box, 0, 0, max_w_ - waterway_length_warning_->get_w(), max_w_ * 2 / 3, 1,
	                   1, std::numeric_limits<int32_t>::max(), UI::PanelStyle::kWui, std::string(),
	                   UI::SpinBox::Units::kFields);
	/** TRANSLATORS: Map Options: Waterways are disabled */
	waterway_length_box_->add_replacement(1, _("Disabled"));
	waterway_length_box_->changed.connect([this]() { update_waterway_length_warning(); });
	ww_box->add(waterway_length_warning_, UI::Box::Resizing::kFullSize);
	ww_box->add_inf_space();
	ww_box->add(waterway_length_box_, UI::Box::Resizing::kFullSize);
	tags_box_.add(ww_box, UI::Box::Resizing::kFullSize);
	tags_box_.add_space(padding_);

	inner_teams_box_.set_force_scrolling(true);
	inner_teams_box_.set_scrollbar_style(UI::PanelStyle::kWui);
	for (const Widelands::SuggestedTeamLineup& team : parent.egbase().map().get_suggested_teams()) {
		SuggestedTeamsEntry* ste = new SuggestedTeamsEntry(this, &inner_teams_box_, parent.egbase().map(), max_w_ - UI::Scrollbar::kSize, team);
		inner_teams_box_.add(ste);
		inner_teams_box_.add_space(kSuggestedTeamsUnitSize);
		suggested_teams_entries_.push_back(ste);
	}

	teams_box_.add(new UI::Textarea(&teams_box_, 0, 0, max_w_, labelh_, _("Suggested Teams:")));
	teams_box_.add(&inner_teams_box_, UI::Box::Resizing::kExpandBoth);
	teams_box_.add(&new_suggested_team_, UI::Box::Resizing::kFullSize);
	unsigned int nr_players = static_cast<unsigned int>(eia().egbase().map().get_nrplayers());
	std::string players =
	   (boost::format(ngettext("%u Player", "%u Players", nr_players)) % nr_players).str();
	teams_box_.add(new UI::Textarea(&teams_box_, 0, 0, max_w_, labelh_, players));
	new_suggested_team_.sigclicked.connect([this]() {
		SuggestedTeamsEntry* ste = new SuggestedTeamsEntry(this, &inner_teams_box_, eia().egbase().map(), max_w_ - UI::Scrollbar::kSize, Widelands::SuggestedTeamLineup());
		inner_teams_box_.add(ste);
		inner_teams_box_.add_space(kSuggestedTeamsUnitSize);
		suggested_teams_entries_.push_back(ste);
	});

	tab_box_.add(&tabs_, UI::Box::Resizing::kFullSize);
	tabs_.add("main_map_options", g_gr->images().get("images/wui/menus/toggle_minimap.png"),
	          &main_box_, _("Main Options"));
	tabs_.add("map_tags", g_gr->images().get("images/ui_basic/checkbox_checked.png"), &tags_box_,
	          _("Tags"));
	tabs_.add("map_teams", g_gr->images().get("images/wui/editor/tools/players.png"), &teams_box_,
	          _("Teams"));

	name_.changed.connect([this]() { changed(); });
	author_.changed.connect([this]() { changed(); });
	descr_->changed.connect([this]() { changed(); });
	hint_->changed.connect([this]() { changed(); });
	waterway_length_box_->changed.connect([this]() { changed(); });
	for (const auto& tag : tags_checkboxes_) {
		tag.second->changed.connect([this]() { changed(); });
	}

	balancing_dropdown_.selected.connect([this] { changed(); });

	ok_.sigclicked.connect([this]() { clicked_ok(); });
	cancel_.sigclicked.connect([this]() { clicked_cancel(); });

	update();
	ok_.set_enabled(true);

	name_.focus();
	center_to_parent();
	move_to_top();
}

void MainMenuMapOptions::update_waterway_length_warning() {
	const uint32_t len = waterway_length_box_->get_value();
	if (len > kMaxRecommendedWaterwayLengthLimit) {
		waterway_length_warning_->set_icon(g_gr->images().get("images/ui_basic/stop.png"));
		waterway_length_warning_->set_tooltip(
		   (boost::format(_("It is not recommended to permit waterway lengths greater than %u")) %
		    kMaxRecommendedWaterwayLengthLimit)
		      .str());
	} else {
		waterway_length_warning_->set_icon(nullptr);
		waterway_length_warning_->set_tooltip("");
	}
}

/**
 * Updates all UI::Textareas in the UI::Window to represent currently
 * set values
 */
void MainMenuMapOptions::update() {
	const Widelands::Map& map = eia().egbase().map();
	author_.set_text(map.get_author());
	name_.set_text(map.get_name());
	size_.set_text((boost::format(_("Size: %1% x %2%")) % map.get_width() % map.get_height()).str());
	descr_->set_text(map.get_description());
	hint_->set_text(map.get_hint());
	waterway_length_box_->set_value(map.get_waterway_max_length());
	update_waterway_length_warning();

	std::set<std::string> tags = map.get_tags();
	for (auto tag : tags_checkboxes_) {
		tag.second->set_state(tags.count(tag.first) > 0);
	}

	balancing_dropdown_.select(tags.count("balanced") ? "balanced" : "unbalanced");
}

/**
 * Called when one of the editboxes are changed
 */
void MainMenuMapOptions::changed() {
	ok_.set_enabled(true);
}

void MainMenuMapOptions::clicked_ok() {
	Widelands::Map& map = *eia().egbase().mutable_map();
	map.set_name(name_.text());
	map.set_author(author_.text());
	set_config_string("realname", author_.text());
	map.set_description(descr_->get_text());
	map.set_hint(hint_->get_text());
	map.set_waterway_max_length(waterway_length_box_->get_value());

	map.get_suggested_teams().clear();
	for (SuggestedTeamsEntry* ste : suggested_teams_entries_) {
		map.get_suggested_teams().push_back(ste->team());
	}

	map.clear_tags();
	for (const auto& tag : tags_checkboxes_) {
		if (tag.second->get_state()) {
			map.add_tag(tag.first);
		}
	}
	map.add_tag(balancing_dropdown_.get_selected());
	Notifications::publish(NoteMapOptions());
	registry_.destroy();
}

void MainMenuMapOptions::clicked_cancel() {
	registry_.destroy();
}

/*
 * Add a tag to the checkboxes
 */
void MainMenuMapOptions::add_tag_checkbox(UI::Box* parent,
                                          std::string tag,
                                          std::string displ_name) {
	UI::Box* box = new UI::Box(parent, 0, 0, UI::Box::Horizontal, max_w_, checkbox_space_, 0);
	UI::Checkbox* cb = new UI::Checkbox(box, Vector2i::zero(), displ_name);
	box->add(cb, UI::Box::Resizing::kFullSize);
	box->add_space(checkbox_space_);
	parent->add(box);
	parent->add_space(padding_);
	tags_checkboxes_[tag] = cb;
}

void MainMenuMapOptions::delete_suggested_team(SuggestedTeamsEntry* ste) {
	inner_teams_box_.set_force_scrolling(false);
	inner_teams_box_.clear();
	const size_t nr = suggested_teams_entries_.size();
	for (size_t i = 0; i < nr; ++i) {
		if (suggested_teams_entries_[i] == ste) {
			for (size_t j = i + 1; j < nr; ++j) {
				inner_teams_box_.add(suggested_teams_entries_[j]);
				inner_teams_box_.add_space(kSuggestedTeamsUnitSize);
				suggested_teams_entries_[j - 1] = suggested_teams_entries_[j];
			}
			suggested_teams_entries_.resize(nr - 1);
			inner_teams_box_.set_force_scrolling(true);
			return ste->die();
		} else {
			inner_teams_box_.add(suggested_teams_entries_[i]);
			inner_teams_box_.add_space(kSuggestedTeamsUnitSize);
		}
	}
	NEVER_HERE();
}
