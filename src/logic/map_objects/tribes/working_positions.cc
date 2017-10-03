/*
 * Copyright (C) 2006-2017 by the Widelands Development Team
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include "logic/map_objects/tribes/working_positions.h"

namespace Widelands {

WorkingPositions::WorkingPositions(const BillOfMaterials& working_positions,
                                   PlayerImmovable* const immovable,
                                   WorkerArrivedCallback worker_arrived_callback)
   : working_positions_description_(working_positions), player_immovable_(immovable) {
	uint32_t num_slots = 0;
	for (const auto& working_pos : working_positions) {
		num_slots += working_pos.second;
	}
	working_positions_.resize(num_slots);
}

int WorkingPositions::num_slots() const {
	return working_positions_.size();
}

int WorkingPositions::occupied() const {
	int result = 0;
	for (const auto& position : working_positions_) {
		result += position.worker == nullptr ? 0 : 1;
	}
	return result;
}

void WorkingPositions::init(EditorGameBase& egbase) {
	//  Request missing workers.
	auto wp = working_positions_.begin();
	for (const auto& temp_wp : working_positions_description_) {
		DescriptionIndex const worker_index = temp_wp.first;
		for (uint32_t j = temp_wp.second; j; --j, ++wp) {
			if (Worker* const worker = wp->worker) {
				worker->set_location(player_immovable_);
			} else {
				wp->worker_request = &request_worker(worker_index);
			}
		}
	}
}

void WorkingPositions::set_economy(Economy* e) {
	for (auto& working_position : working_positions_) {
		if (working_position.worker_request != nullptr) {
			working_position.worker_request->set_economy(e);
		}
	}
}

void WorkingPositions::cleanup(EditorGameBase& egbase) {
	for (auto& working_position : working_positions_) {
		// NOCOM(#sirver): change worker_request to unique ptr?
		delete working_position.worker_request;
		working_position.worker_request = nullptr;
		Worker* const w = working_position.worker;

		//  Ensure we do not re-request the worker when remove_worker is called.
		working_position.worker = nullptr;

		// Actually remove the worker
		if (egbase.objects().object_still_available(w))
			w->set_location(nullptr);
	}
}
// NOCOM(#sirver): Fix non-const references

Request& WorkingPositions::request_worker(DescriptionIndex const wareid) {
	return *new Request(
	   *player_immovable_, wareid, WorkingPositions::request_worker_callback, wwWORKER);
}


// NOCOM(#sirver): return an enum?
bool WorkingPositions::warp_worker(const WorkerDescr& worker_descr, EditorGameBase* egbase) {
	bool assigned = false;
	for (auto& working_position : working_positions_) {
		if (working_position.worker)
			continue;

		assert(working_position.worker_request);
		if (working_position.worker_request->get_index() != worker_descr.worker_index())
			continue;

		// Okay, space is free and worker is fitting. Let's create him
		Worker& worker = worker_descr.create(
		   *egbase, player_immovable_->owner(), player_immovable_, player_immovable_->get_position());

		if (upcast(Game, game, egbase))
			worker.start_task_idle(*game, 0, -1);
		working_position.worker = &worker;
		delete working_position.worker_request;
		working_position.worker_request = nullptr;
		assigned = true;
		break;
	}
	return assigned;
}

// NOCOM(#sirver): change parameter type
void WorkingPositions::remove_worker(Worker& w) {
	auto wp = working_positions_.begin();

	for (const auto& temp_wp : working_positions_description_) {
		DescriptionIndex const worker_index = temp_wp.first;
		for (uint32_t j = temp_wp.second; j; --j, ++wp) {
			Worker* const worker = wp->worker;
			if (worker && worker == &w) {
				// do not request the type of worker that is currently assigned -
				// maybe a trained worker was evicted to make place for a level 0
				// worker. Therefore we again request the worker from the
				// WorkingPosition of descr()
				wp->worker = nullptr;
				wp->worker_request = &request_worker(worker_index);
				return;
			}
		}
	}
}

void WorkingPositions::request_worker_callback(
   Game& game, Request& rq, DescriptionIndex /* widx */, Worker* const w, PlayerImmovable& target) {
	// NOCOM(#sirver): this does not work moving forward.
	auto* wp = dynamic_cast<ProductionSite&>(target)->working_positions();
	assert(w != nullptr);

	// If there is more than one working position, it's possible, that different level workers are
	// requested and therefor possible, that a higher qualified worker answers a request for a lower
	// leveled worker, although a worker with equal level (as the arrived worker has) is needed as
	// well.
	// Therefor, we first check whether the worker exactly fits the requested one. If yes, we place
	// the
	// worker and everything is fine, else we shuffle through the working positions, whether one of
	// them
	// needs a worker like the one just arrived. That way it is of course still possible, that the
	// worker is
	// placed on the slot that originally requested the arrived worker.
	bool worker_placed = false;
	DescriptionIndex idx = w->descr().worker_index();
	for (WorkingPosition* wp = wp->working_positions_;; ++wp) {
		if (wp->worker_request == &rq) {
			if (wp->worker_request->get_index() == idx) {
				// Place worker
				delete &rq;
				*wp = WorkingPosition(nullptr, w);
				worker_placed = true;
			} else {
				// Set new request for this slot
				DescriptionIndex workerid = wp->worker_request->get_index();
				delete wp->worker_request;
				wp->worker_request = &wp->request_worker(workerid);
			}
			break;
		}
	}
	while (!worker_placed) {
		{
			for (auto& wp : working_positions_) {
				// Find a fitting slot
				if (!wp.worker && !worker_placed)
					if (wp.worker_request->get_index() == idx) {
						delete wp.worker_request;
						*wp = WorkingPosition(nullptr, w);
						worker_placed = true;
						break;
					}
			}
		}
		if (!worker_placed) {
			// Find the next smaller version of this worker
			DescriptionIndex nuwo = game.tribes().nrworkers();
			DescriptionIndex current = 0;
			for (; current < nuwo; ++current) {
				WorkerDescr const* worker = game.tribes().get_worker_descr(current);
				if (worker->becomes() == idx) {
					idx = current;
					break;
				}
			}
			if (current == nuwo)
				throw wexception(
				   "Something went wrong! No fitting place for worker %s in %s at (%u, %u) found!",
				   w->descr().descname().c_str(), wp->player_immovable_.descr().descname().c_str(),
				   wp->player_immovable_.get_position().x, wp->player_immovable_.get_position().y);
		}
	}

	worker_arrived_callback_(&game, w);
}

Worker* WorkingPositions::primary_worker() {
	return working_positions_[0].worker;
}

void WorkingPositions::train_workers(Game* game) {
	for (uint32_t i = descr().nr_working_positions(); i;)
		working_positions_[--i].worker->gain_experience(game);
}

void WorkingPositions::request_worker_callback(Game& game,
	                                    Request& rq,
	                                    DescriptionIndex /* widx */,
	                                    Worker* const w,
	                                    PlayerImmovable& target) {
}

}  // namespace Widelands
