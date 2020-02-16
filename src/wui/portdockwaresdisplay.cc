/*
 * Copyright (C) 2011-2019 by the Widelands Development Team
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

#include "wui/portdockwaresdisplay.h"

#include <boost/lexical_cast.hpp>

#include "economy/expedition_bootstrap.h"
#include "economy/portdock.h"
#include "graphic/text_layout.h"
#include "logic/player.h"
#include "ui_basic/icon.h"
#include "wui/inputqueuedisplay.h"

using Widelands::InputQueue;
using Widelands::PortDock;
using Widelands::Warehouse;

namespace {

/**
 * Display wares or workers that are waiting to be shipped from a port.
 */
struct PortDockWaresDisplay : AbstractWaresDisplay {
	PortDockWaresDisplay(Panel* parent,
	                     uint32_t width,
	                     const PortDock& pd,
	                     Widelands::WareWorker type);

	std::string info_for_ware(Widelands::DescriptionIndex ware) override;

private:
	const PortDock& portdock_;
};

PortDockWaresDisplay::PortDockWaresDisplay(Panel* parent,
                                           uint32_t width,
                                           const PortDock& pd,
                                           Widelands::WareWorker type)
   : AbstractWaresDisplay(parent, 0, 0, pd.owner().tribe(), type, false), portdock_(pd) {
	set_inner_size(width, 0);
}

std::string PortDockWaresDisplay::info_for_ware(Widelands::DescriptionIndex ware) {
	const uint32_t count = portdock_.count_waiting(get_type(), ware);
	return boost::lexical_cast<std::string>(count);
}

}  // anonymous namespace

/**
 * Create a panel that displays the wares or workers that are waiting to be shipped from a port.
 */
AbstractWaresDisplay* create_portdock_wares_display(UI::Panel* parent,
                                                    uint32_t width,
                                                    const PortDock& pd,
                                                    Widelands::WareWorker type) {
	return new PortDockWaresDisplay(parent, width, pd, type);
}

constexpr uint32_t kItemSize = 28;
constexpr uint32_t kAdditionalItemsBorder = 4;

static char const* kPicWarePresent = "images/wui/buildings/high_priority_button.png";
static char const* kPicWareComing = "images/wui/buildings/normal_priority_button.png";
static char const* kPicWareMissing = "images/wui/buildings/low_priority_button.png";
static char const* kNoWare = "images/wui/editor/no_ware.png";
static const auto kEmptySlot = std::make_pair(Widelands::wwWARE, Widelands::INVALID_INDEX);

struct PortDockAdditionalItemsDisplay : UI::Box {
public:
	PortDockAdditionalItemsDisplay(
	   Widelands::Game& g, Panel* parent, bool can_act, PortDock& pd, const uint32_t capacity)
	   : UI::Box(parent, 0, 0, UI::Box::Horizontal), game_(g), portdock_(pd), capacity_(capacity) {
		assert(capacity_ > 0);
		assert(portdock_.expedition_bootstrap());
		assert(portdock_.expedition_bootstrap()->count_additional_queues() <= capacity_);
		for (uint32_t c = 0; c < capacity_; ++c) {
			UI::Box* box = new UI::Box(this, 0, 0, UI::Box::Vertical);

			UI::Dropdown<std::pair<Widelands::WareWorker, Widelands::DescriptionIndex>>& d =
			   *new UI::Dropdown<std::pair<Widelands::WareWorker, Widelands::DescriptionIndex>>(
			      box, (boost::format("additional_%u") % c).str(), 0, 0, kWareMenuPicWidth, 8,
			      kWareMenuPicHeight, "", UI::DropdownType::kPictorial, UI::PanelStyle::kWui,
			      UI::ButtonStyle::kWuiSecondary);
			d.add(_("(Empty)"), kEmptySlot, g_gr->images().get(kNoWare), true, _("(Empty)"));
			for (Widelands::DescriptionIndex i : pd.owner().tribe().wares()) {
				const Widelands::WareDescr& w = *pd.owner().tribe().get_ware_descr(i);
				d.add(
				   w.descname(), std::make_pair(Widelands::wwWARE, i), w.icon(), false, w.descname());
			}
			for (Widelands::DescriptionIndex i : pd.owner().tribe().workers()) {
				const Widelands::WorkerDescr& w = *pd.owner().tribe().get_worker_descr(i);
				d.add(
				   w.descname(), std::make_pair(Widelands::wwWORKER, i), w.icon(), false, w.descname());
			}
			d.set_enabled(can_act);
			d.selected.connect(boost::bind(&PortDockAdditionalItemsDisplay::select, this, c));

			UI::Icon* icon = new UI::Icon(box, g_gr->images().get(kNoWare));
			boxes_.push_back(box);
			icons_.push_back(icon);
			dropdowns_.push_back(&d);
			box->add(&d);
			box->add(icon);
			add(box);
		}
		update_selection();
	}

	void think() override {
		UI::Box::think();
		if (!portdock_.expedition_bootstrap()) {
			return die();
		}
		update_selection();

		for (uint32_t c = 0; c < capacity_; ++c) {
			const InputQueue* iq = portdock_.expedition_bootstrap()->inputqueue(c);
			assert(!iq || (iq->get_max_size() == 1 && iq->get_max_fill() == 1));
			icons_[c]->set_icon(g_gr->images().get(
			   iq ?
			      iq->get_filled() ? kPicWarePresent : iq->get_missing() ? kPicWareMissing :
			                                                               kPicWareComing :
			      kNoWare));
			icons_[c]
			   ->set_tooltip(iq ?
			                    /** TRANSLATORS: Tooltip for a ware that is present in the building */
			                    iq->get_filled() ?
			                    _("Present") :
			                    /** TRANSLATORS: Tooltip for a ware that is neither present in the
			                       building nor being transported there */
			                       iq->get_missing() ?
			                    _("Missing") :
			                    /** TRANSLATORS: Tooltip for a ware that is not present in the
			                       building, but already being transported there */
			                          _("Coming") :
			                    "");
		}
	}

	void update_selection() {
		for (uint32_t c = 0; c < capacity_; ++c) {
			if (dropdowns_[c]->is_expanded()) {
				continue;
			}
			const InputQueue* iq = portdock_.expedition_bootstrap()->inputqueue(c);
			if (!iq) {
				dropdowns_[c]->select(kEmptySlot);
			} else {
				dropdowns_[c]->select(std::make_pair(iq->get_type(), iq->get_index()));
			}
			assert(dropdowns_[c]->has_selection());
		}
	}

	void select(uint32_t index) {
		assert(index < dropdowns_.size());
		const auto& new_sel = dropdowns_[index]->get_selected();
		const InputQueue* iq = portdock_.expedition_bootstrap()->inputqueue(index);
		if (new_sel.second == Widelands::INVALID_INDEX && !iq) {
			return;
		}
		if (iq && iq->get_type() == new_sel.first && iq->get_index() == new_sel.second) {
			return;
		}
		if (iq) {
			game_.send_player_expedition_config(portdock_, iq->get_type(), iq->get_index(), false);
		}
		if (new_sel.second != Widelands::INVALID_INDEX) {
			game_.send_player_expedition_config(portdock_, new_sel.first, new_sel.second, true);
		}
	}

private:
	Widelands::Game& game_;
	PortDock& portdock_;
	uint32_t capacity_;
	std::vector<UI::Box*> boxes_;
	std::vector<UI::Icon*> icons_;
	std::vector<UI::Dropdown<std::pair<Widelands::WareWorker, Widelands::DescriptionIndex>>*>
	   dropdowns_;
};

/// Create a panel that displays the wares and the builder waiting for the expedition to start.
UI::Box*
create_portdock_expedition_display(UI::Panel* parent, Warehouse& wh, InteractiveGameBase& igb) {
	UI::Box& box = *new UI::Box(parent, 0, 0, UI::Box::Vertical);

	// Add the input queues.
	int32_t capacity =
	   igb.egbase().tribes().get_ship_descr(wh.get_owner()->tribe().ship())->get_default_capacity();
	for (const InputQueue* wq : wh.get_portdock()->expedition_bootstrap()->queues(false)) {
		InputQueueDisplay* iqd = new InputQueueDisplay(&box, 0, 0, igb, wh, *wq, true);
		box.add(iqd);
		capacity -= wq->get_max_size();
	}
	assert(capacity >= 0);

	if (capacity > 0) {
		const bool can_act = igb.can_act(wh.get_owner()->player_number());
		box.add(new PortDockAdditionalItemsDisplay(
		           igb.game(), &box, can_act, *wh.get_portdock(), capacity),
		        UI::Box::Resizing::kAlign, UI::Align::kCenter);
	}

	return &box;
}
