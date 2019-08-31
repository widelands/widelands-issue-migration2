/*
 * Copyright (C) 2004-2019 by the Widelands Development Team
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

#include "economy/economy_data_packet.h"

#include "economy/economy.h"
#include "io/fileread.h"
#include "io/filewrite.h"
#include "logic/map_objects/tribes/tribe_descr.h"
#include "logic/player.h"
#include "map_io/map_object_loader.h"
#include "map_io/map_object_saver.h"

constexpr uint16_t kCurrentPacketVersion = 5;

namespace Widelands {

void EconomyDataPacket::read(FileRead& fr) {
	try {
		uint16_t const packet_version = fr.unsigned_16();
		if (packet_version <= kCurrentPacketVersion && packet_version >= 4) {
			const Serial saved_serial = fr.unsigned_32();
			if (eco_->serial_ != saved_serial) {
				throw GameDataError(
				   "Representative flag/ship has economy serial %d, but the data packet has %d",
				   eco_->serial_, saved_serial);
			}
			// TODO(Nordfriese): Savegame compatibility
			// NOCOM removed on purpose? assert(Economy::last_economy_serial_ >= eco_->serial_);
			assert((packet_version == kCurrentPacketVersion) ^ (mol_ != nullptr));
			Economy* other_eco = nullptr;
			if (mol_) {
				assert(eco_->type() == wwWARE);
				const Serial serial = mol_->get_existing_economy_savegame_compatibility(eco_->serial());
				other_eco = eco_->owner().get_economy(serial);
				assert(other_eco);
				assert(other_eco->type() == wwWORKER);
			}
			try {
				const TribeDescr& tribe = eco_->owner().tribe();
				while (Time const last_modified = fr.unsigned_32()) {
					char const* const type_name = fr.c_string();
					uint32_t const permanent = fr.unsigned_32();
					DescriptionIndex i;
					switch (eco_->type()) {
						case wwWARE:
							i = tribe.ware_index(type_name);
							if (tribe.has_ware(i)) {
								if (tribe.get_ware_descr(i)->default_target_quantity(tribe.name()) ==
									kInvalidWare) {
									log("WARNING: target quantity configured for ware %s, "
										"which should not have target quantity, "
										"ignoring\n",
										type_name);
								} else {
									Economy::TargetQuantity& tq = eco_->target_quantities_[i];
									if (tq.last_modified) {
										throw GameDataError("duplicated entry for ware %s", type_name);
									}
									tq.permanent = permanent;
									tq.last_modified = last_modified;
								}
							} else if (other_eco) {
								i = tribe.worker_index(type_name);
								if (tribe.has_worker(i)) {
									if (tribe.get_worker_descr(i)->default_target_quantity() == kInvalidWare) {
										log("WARNING: target quantity configured for worker %s, "
											"which should not have target quantity, "
											"ignoring\n",
											type_name);
									} else {
										Economy::TargetQuantity& tq = other_eco->target_quantities_[i];
										if (tq.last_modified) {
											throw GameDataError("duplicated entry for worker %s", type_name);
										}
										tq.permanent = permanent;
										tq.last_modified = last_modified;
									}
								} else {
									log("WARNING: target quantity configured for \"%s\" in worker economy, "
										"which is not a worker type defined in tribe "
										"%s, ignoring\n",
										type_name, tribe.name().c_str());
								}
							} else {
								log("WARNING: target quantity configured for \"%s\" in ware economy, "
									"which is not a ware type defined in tribe "
									"%s, ignoring\n",
									type_name, tribe.name().c_str());
							}
							break;
						case wwWORKER:
							i = tribe.worker_index(type_name);
							if (tribe.has_worker(i)) {
								if (tribe.get_worker_descr(i)->default_target_quantity() == kInvalidWare) {
									log("WARNING: target quantity configured for worker %s, "
										"which should not have target quantity, "
										"ignoring\n",
										type_name);
								} else {
									Economy::TargetQuantity& tq = eco_->target_quantities_[i];
									if (tq.last_modified) {
										throw GameDataError("duplicated entry for worker %s", type_name);
									}
									tq.permanent = permanent;
									tq.last_modified = last_modified;
								}
							} else {
								log("WARNING: target quantity configured for \"%s\" in worker economy, "
									"which is not a worker type defined in tribe "
									"%s, ignoring\n",
									type_name, tribe.name().c_str());
							}
							break;
					}
				}
			} catch (const WException& e) {
				throw GameDataError("target quantities: %s", e.what());
			}
			eco_->request_timerid_ = fr.unsigned_32();
			if (other_eco) {
				other_eco->request_timerid_ = eco_->request_timerid_;
			}
		} else {
			throw UnhandledVersionError("EconomyDataPacket", packet_version, kCurrentPacketVersion);
		}
	} catch (const std::exception& e) {
		throw GameDataError("economy: %s", e.what());
	}
}

void EconomyDataPacket::write(FileWrite& fw) {
	assert(!mol_);
	fw.unsigned_16(kCurrentPacketVersion);

	// We save the serial number for sanity checks
	fw.unsigned_32(eco_->serial());

	// Requests etc.
	const TribeDescr& tribe = eco_->owner().tribe();
	for (const DescriptionIndex& w_index : (eco_->type() == wwWARE ? tribe.wares() : tribe.workers())) {
		const Economy::TargetQuantity& tq = eco_->target_quantities_[w_index];
		if (Time const last_modified = tq.last_modified) {
			fw.unsigned_32(last_modified);
			if (eco_->type() == wwWARE) {
				fw.c_string(tribe.get_ware_descr(w_index)->name());
			} else {
				fw.c_string(tribe.get_worker_descr(w_index)->name());
			}
			fw.unsigned_32(tq.permanent);
		}
	}
	fw.unsigned_32(0);  //  terminator
	fw.unsigned_32(eco_->request_timerid_);
}
}  // namespace Widelands
