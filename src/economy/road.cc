/*
 * Copyright (C) 2004-2018 by the Widelands Development Team
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

#include "economy/road.h"

#include "base/macros.h"
#include "economy/economy.h"
#include "economy/flag.h"
#include "economy/request.h"
#include "logic/editor_game_base.h"
#include "logic/game.h"
#include "logic/map_objects/map_object.h"
#include "logic/map_objects/tribes/carrier.h"
#include "logic/map_objects/tribes/tribe_descr.h"
#include "logic/player.h"

namespace Widelands {

// dummy instance because MapObject needs a description
namespace {
const RoadDescr g_road_descr("road", "Road");
}

bool Road::is_road_descr(MapObjectDescr const* const descr) {
	return descr == &g_road_descr;
}

Road::CarrierSlot::CarrierSlot() : carrier(nullptr), carrier_request(nullptr), second_carrier(false) {
}

/**
 * Most of the actual work is done in init.
 */
Road::Road()
   : RoadBase(g_road_descr, RoadType::kNone), wallet_(0), last_wallet_charge_(0) {
	CarrierSlot slot;
    carrier_slots_.push_back(slot);
    carrier_slots_.push_back(slot);
	carrier_slots_[0].second_carrier = false;
	carrier_slots_[1].second_carrier = true;
}

/**
 * Most of the actual work is done in cleanup.
 */
Road::~Road() {
	for (CarrierSlot& slot : carrier_slots_) {
		delete slot.carrier_request;
	}
}

/**
 * Create a road between the given flags, using the given path.
 */
Road& Road::create(EditorGameBase& egbase, Flag& start, Flag& end, const Path& path) {
	assert(start.get_position() == path.get_start());
	assert(end.get_position() == path.get_end());
	assert(start.get_owner() == end.get_owner());

	Road& road = *new Road();
	road.set_owner(start.get_owner());
	road.type_ = RoadType::kNormal;
	road.flags_[FlagStart] = &start;
	road.flags_[FlagEnd] = &end;
	// flagidx_ is set when attach_road() is called, i.e. in init()
	road.set_path(egbase, path);

	road.init(egbase);

	return road;
}

void Road::cleanup(EditorGameBase& egbase) {
	for (CarrierSlot& slot : carrier_slots_) {
		delete slot.carrier_request;
		slot.carrier_request = nullptr;

		// carrier will be released via PlayerImmovable::cleanup
		slot.carrier = nullptr;
	}
	RoadBase::cleanup(egbase);
}

void Road::link_into_flags(EditorGameBase& egbase) {
	RoadBase::link_into_flags(egbase);
	if (upcast(Game, game, &egbase)) {
		for (CarrierSlot& slot : carrier_slots_) {
			if (Carrier* const carrier = slot.carrier.get(*game)) {
				// This happens after a road split. Tell the carrier what's going on.
				carrier->set_location(this);
				carrier->update_task_road(*game);
			} else if (!slot.carrier_request && (!slot.second_carrier || get_roadtype() == RoadType::kBusy)) {
				// Normal carriers are requested at once, second carriers only for busy roads
				request_carrier(slot);
			}
		}
	}
}

void Road::set_economy(Economy* const e) {
	RoadBase::set_economy(e);
	for (CarrierSlot& slot : carrier_slots_) {
		if (slot.carrier_request) {
			slot.carrier_request->set_economy(e);
		}
	}
}

/**
 * Request a new carrier.
 *
 * Only call this if the road can handle a new carrier, and if no request has
 * been issued.
 */
void Road::request_carrier(CarrierSlot& slot) {
	slot.carrier_request = new Request(*this, slot.second_carrier ? owner().tribe().carrier2() :
			owner().tribe().carrier(), request_carrier_callback, wwWORKER);
}

/**
 * The carrier has arrived successfully.
 */
void Road::request_carrier_callback(
   Game& game, Request& rq, DescriptionIndex, Worker* const w, PlayerImmovable& target) {
	assert(w);

	Road& road = dynamic_cast<Road&>(target);

	for (CarrierSlot& slot : road.carrier_slots_) {
		if (slot.carrier_request == &rq) {
			Carrier& carrier = dynamic_cast<Carrier&>(*w);
			slot.carrier_request = nullptr;
			slot.carrier = &carrier;

			carrier.start_task_road(game);
			delete &rq;
			return;
		}
	}

	/*
	 * Oops! We got a request_callback but don't have the request.
	 * Try to send him home.
	 */
	log("Road(%u): got a request_callback but do not have the request\n", road.serial());
	delete &rq;
	w->start_task_gowarehouse(game);
}
uint8_t Road::carriers_count() const {
	return (carrier_slots_[1].carrier == nullptr) ? 1 : 2;
}

/**
 * If we lost our carrier, re-request it.
 */
void Road::remove_worker(Worker& w) {
	EditorGameBase& egbase = get_owner()->egbase();

	for (CarrierSlot& slot : carrier_slots_) {
		Carrier const* carrier = slot.carrier.get(egbase);

		if (carrier == &w) {
			slot.carrier = nullptr;
			carrier = nullptr;
			request_carrier(slot);
		}
	}

	PlayerImmovable::remove_worker(w);
}

/**
 * A carrier was created by someone else (e.g. Scripting Engine)
 * and should now be assigned to this road.
 */
void Road::assign_carrier(Carrier& c, uint8_t slot) {
	assert(slot <= 1);

	// Send the worker home if it occupies our slot
	CarrierSlot& s = carrier_slots_[slot];

	delete s.carrier_request;
	s.carrier_request = nullptr;
	if (Carrier* const current_carrier = s.carrier.get(owner().egbase()))
		current_carrier->set_location(nullptr);

	carrier_slots_[slot].carrier = &c;
	carrier_slots_[slot].carrier_request = nullptr;
}

/**
 * The flag that splits this road has been initialized. Perform the actual
 * splitting.
 *
 * After the split, this road will span [start...new flag]. A new road will
 * be created to span [new flag...end]
 */
// TODO(SirVer): This needs to take an EditorGameBase as well.
void Road::postsplit(Game& game, Flag& flag) {
	Flag& oldend = *flags_[FlagEnd];

	// detach from end
	oldend.detach_road(flagidx_[FlagEnd]);

	// build our new path and the new road's path
	const Map& map = game.map();
	CoordPath path(map, path_);
	CoordPath secondpath(path);
	int32_t const index = path.get_index(flag.get_position());

	assert(index > 0);
	assert(static_cast<uint32_t>(index) < path.get_nsteps() - 1);

	path.truncate(index);
	secondpath.trim_start(index);

	molog("splitting road: first part:\n");
	for (const Coords& coords : path.get_coords()) {
		molog("* (%i, %i)\n", coords.x, coords.y);
	}
	molog("                second part:\n");
	for (const Coords& coords : secondpath.get_coords()) {
		molog("* (%i, %i)\n", coords.x, coords.y);
	}

	// change road size and reattach
	flags_[FlagEnd] = &flag;
	set_path(game, path);

	const Direction dir = get_reverse_dir(path_[path_.get_nsteps() - 1]);
	flags_[FlagEnd]->attach_road(dir, this);
	flagidx_[FlagEnd] = dir;

	// recreate road markings
	mark_map(game);

	// create the new road
	Road& newroad = *new Road();
	newroad.set_owner(get_owner());
	newroad.type_ = type_;
	newroad.flags_[FlagStart] = &flag;  //  flagidx will be set on init()
	newroad.flags_[FlagEnd] = &oldend;
	newroad.set_path(game, secondpath);

	// Find workers on this road that need to be reassigned
	// The algorithm is pretty simplistic, and has a bias towards keeping
	// the worker around; there's obviously nothing wrong with that.

	std::vector<Worker*> const workers = get_workers();
	std::vector<Worker*> reassigned_workers;

	for (Worker* w : workers) {
		int32_t idx = path.get_index(w->get_position());

		// Careful! If the worker is currently inside the building at our
		// starting flag, we *must not* reassign him.
		// If he is in the building at our end flag or at the other road's
		// end flag, he can be reassigned to the other road.
		if (idx < 0) {
			if (dynamic_cast<Building const*>(map.get_immovable(w->get_position()))) {
				Coords pos;
				map.get_brn(w->get_position(), &pos);
				if (pos == path.get_start())
					idx = 0;
			}
		}

		if (idx < 0) {
			reassigned_workers.push_back(w);

			/*
			 * The current worker is not on this road. Search him
			 * in this road and remove him. Than add him to the new road
			 */
			for (CarrierSlot& old_slot : carrier_slots_) {
				Carrier const* const carrier = old_slot.carrier.get(game);

				if (carrier == w) {
					old_slot.carrier = nullptr;
					for (CarrierSlot& new_slot : newroad.carrier_slots_) {
						if (!new_slot.carrier.get(game) && !new_slot.carrier_request &&
						    new_slot.second_carrier == old_slot.second_carrier) {
							upcast(Carrier, new_carrier, w);
							new_slot.carrier = new_carrier;
							break;
						}
					}
				}
			}
		}

		// Cause a worker update in any case
		w->send_signal(game, "road");
	}

	// Initialize the new road
	newroad.init(game);
	newroad.wallet_ = wallet_;

	// Actually reassign workers after the new road has initialized,
	// so that the reassignment is safe
	for (Worker*& w : reassigned_workers) {
		w->set_location(&newroad);
	}

	//  Request a new carrier for this road if necessary. This must be done
	//  _after_ the new road initializes, otherwise request routing might not
	//  work correctly
	for (CarrierSlot& slot : carrier_slots_) {
		if (!slot.carrier.get(game) && !slot.carrier_request &&
		    (!slot.second_carrier || type_ == RoadType::kBusy)) {
			request_carrier(slot);
		}
	}

	//  Make sure wares waiting on the original endpoint flags are dealt with.
	flags_[FlagStart]->update_wares(game, &oldend);
	oldend.update_wares(game, flags_[FlagStart]);
}

/**
 * Try to pick up a ware from the given flag.
 * \return true if a carrier has been sent on its way, false otherwise.
 */
bool Road::notify_ware(Game& game, Flag& flag) {
	FlagId flagid = &flag == flags_[RoadBase::FlagEnd] ? RoadBase::FlagEnd : RoadBase::FlagStart;
	// Iterate over all carriers and try to find one which will take the ware
	for (CarrierSlot& slot : carrier_slots_) {
		if (Carrier* const carrier = slot.carrier.get(game)) {
			if (carrier->notify_ware(game, flagid)) {
				// The carrier took the ware, so we're done
				return true;
			}
		}
	}
	// No carrier took the ware
	return false;
}

/**
 * Update last_wallet_charge_ with the current gametime.
 */
void Road::update_wallet_chargetime(Game& game) {
	last_wallet_charge_ = game.get_gametime();
}

/**
 * Subtract maintenance cost, and check for demotion.
 */
void Road::charge_wallet(Game& game) {
	const uint32_t current_gametime = game.get_gametime();
	assert(last_wallet_charge_ <= current_gametime);

	wallet_ -= carriers_count() * (current_gametime - last_wallet_charge_) / 1000;
	last_wallet_charge_ = current_gametime;

	if (wallet_ < 0) {
		wallet_ = 0;
		if (type_ == RoadType::kBusy) {
			// Demote the road
			Carrier* const second_carrier = carrier_slots_[1].carrier.get(game);
			if (second_carrier && second_carrier->top_state().task == &Carrier::taskRoad) {
				second_carrier->send_signal(game, "cancel");
				// This signal is not handled in any special way. It will simply pop the task off the
				// stack. The string "cancel" has been used to clarify the final goal we want to
				// achieve, ie: cancelling the current task.
				carrier_slots_[1].carrier = nullptr;
				carrier_slots_[1].carrier_request = nullptr;
				type_ = RoadType::kNormal;
				mark_map(game);
			}
		}
	}
}

int32_t Road::wallet() const {
	return wallet_;
}

void Road::add_to_wallet(int32_t sum) {
	wallet_ += sum;
}

/**
 * Add carrying payment, and check for promotion.
 */
void Road::pay_for_road(Game& game, uint8_t queue_length) {
	wallet_ += 2 * (carriers_count() + 1) * (4 * queue_length + path_.get_nsteps());
	charge_wallet(game);

	if (type_ == RoadType::kNormal && wallet_ > 1.5 * kRoadAnimalPrice) {
		// Promote the road
		wallet_ -= kRoadAnimalPrice;
		type_ = RoadType::kBusy;
		flags_[0]->propagate_promoted_road(this);
		flags_[1]->propagate_promoted_road(this);
		mark_map(game);
		for (CarrierSlot& slot : carrier_slots_) {
			if (!slot.carrier.get(game) && !slot.carrier_request && slot.second_carrier) {
				request_carrier(slot);
			}
		}
	}
	wallet_ = std::min(wallet_, kRoadMaxWallet);
}

/**
 * Add extra coins for street-segment at building.
 */
void Road::pay_for_building() {
	wallet_ += 2 * (carriers_count() + 1);
	// Don't bother with checks here, since the next ware will cause them anyway
}

void Road::log_general_info(const EditorGameBase& egbase) const {
	PlayerImmovable::log_general_info(egbase);
	molog("wallet: %i\n", wallet_);
}
}  // namespace Widelands
