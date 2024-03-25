/*
 * Copyright (C) 2006-2024 by the Widelands Development Team
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

#ifndef WL_LOGIC_TRADE_AGREEMENT_H
#define WL_LOGIC_TRADE_AGREEMENT_H

#include <cstdint>
#include <limits>

#include "logic/map_objects/tribes/bill_of_materials.h"
#include "logic/widelands.h"

namespace Widelands {

class Market;

struct Trade {
	BillOfMaterials items_to_send;
	BillOfMaterials items_to_receive;
	int num_batches;
	OPtr<Market> initiator;
	PlayerNumber receiving_player;
};

struct TradeAgreement {
	enum class State : uint8_t {
		kProposed = 0,
		kRunning = 1,
	};

	State state;
	Trade trade;
	OPtr<Market> receiver;
};

enum class TradeAction : uint8_t {
	kCancel = 0,
	kAccept = 1,
	kReject = 2,
	kRetract = 3,
};

}  // namespace Widelands

#endif  // end of include guard: WL_LOGIC_TRADE_AGREEMENT_H
