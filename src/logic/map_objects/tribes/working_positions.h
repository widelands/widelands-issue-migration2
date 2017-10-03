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

#ifndef WL_LOGIC_MAP_OBJECTS_TRIBES_WORKING_POSITIONS_H
#define WL_LOGIC_MAP_OBJECTS_TRIBES_WORKING_POSITIONS_H

#include <functional>
#include <vector>

#include "economy/request.h"
#include "logic/map_objects/tribes/bill_of_materials.h"
#include "logic/map_objects/tribes/worker.h"

namespace Widelands {

	// NOCOM(#sirver): document class
class WorkingPositions {
public:
	using WorkerArrivedCallback = std::function<void (Game* game, Worker* w)>;

	WorkingPositions(const BillOfMaterials& working_positions,
	                 PlayerImmovable* immovable,
	                 WorkerArrivedCallback callback);

	int num_slots() const;
	int occupied() const;

	void init(EditorGameBase& egbase);
	void cleanup(EditorGameBase& egbase);
	void set_economy(Economy* e);
	bool warp_worker(const WorkerDescr& worker_descr);
	void remove_worker(Worker& w);
	void worker_arrived();
	Worker* primary_worker();
	void train_workers(Game* game);

private:
	static void request_worker_callback(Game& game,
	                                    Request& rq,
	                                    DescriptionIndex /* widx */,
	                                    Worker* const w,
	                                    PlayerImmovable& target);
	Request& request_worker(DescriptionIndex wareid);

	const BillOfMaterials& working_positions_description_;
	PlayerImmovable* const player_immovable_;
	WorkerArrivedCallback worker_arrived_callback_;

	struct WorkingPosition {
		Request* worker_request = nullptr;
		Worker* worker = nullptr;
		};
		std::vector<WorkingPosition> working_positions_;

		DISALLOW_COPY_AND_ASSIGN(WorkingPositions);
};

}  // namespace Widelands

#endif  // end of include guard: WL_LOGIC_MAP_OBJECTS_TRIBES_WORKING_POSITIONS_H
