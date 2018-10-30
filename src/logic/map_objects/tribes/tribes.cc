/*
 * Copyright (C) 2006-2018 by the Widelands Development Team
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

#include "logic/map_objects/tribes/tribes.h"

#include <memory>

#include "base/wexception.h"
#include "graphic/graphic.h"
#include "io/filesystem/layered_filesystem.h"
#include "logic/game_data_error.h"
#include "logic/map_objects/tribes/market.h"
#include "logic/map_objects/tribes/tribe_basic_info.h"

namespace Widelands {
Tribes::Tribes(LuaInterface* lua)
   : buildings_(new DescriptionMaintainer<BuildingDescr>()),
     immovables_(new DescriptionMaintainer<ImmovableDescr>()),
     ships_(new DescriptionMaintainer<ShipDescr>()),
     wares_(new DescriptionMaintainer<WareDescr>()),
     workers_(new DescriptionMaintainer<WorkerDescr>()),
     tribes_(new DescriptionMaintainer<TribeDescr>()),
     lua_(lua) {
}

Tribes::~Tribes() {
}

size_t Tribes::nrbuildings() const {
	return buildings_->size();
}

size_t Tribes::nrtribes() const {
	return tribes_->size();
}

size_t Tribes::nrwares() const {
	return wares_->size();
}

size_t Tribes::nrworkers() const {
	return workers_->size();
}

bool Tribes::ware_exists(const std::string& warename) const {
	return wares_->exists(warename) != nullptr;
}
bool Tribes::ware_exists(DescriptionIndex index) const {
	return wares_->get_mutable(index) != nullptr;
}
bool Tribes::worker_exists(const std::string& workername) const {
	return workers_->exists(workername) != nullptr;
}
bool Tribes::worker_exists(DescriptionIndex index) const {
	return workers_->get_mutable(index) != nullptr;
}
bool Tribes::building_exists(const std::string& buildingname) const {
	return buildings_->exists(buildingname) != nullptr;
}
bool Tribes::building_exists(DescriptionIndex index) const {
	return buildings_->get_mutable(index) != nullptr;
}
bool Tribes::immovable_exists(DescriptionIndex index) const {
	return immovables_->get_mutable(index) != nullptr;
}
bool Tribes::ship_exists(DescriptionIndex index) const {
	return ships_->get_mutable(index) != nullptr;
}
bool Tribes::tribe_exists(DescriptionIndex index) const {
	return tribes_->get_mutable(index) != nullptr;
}

DescriptionIndex Tribes::safe_building_index(const std::string& buildingname) const {
	const DescriptionIndex result = building_index(buildingname);
	if (!building_exists(result)) {
		throw GameDataError("Unknown building type \"%s\"", buildingname.c_str());
	}
	return result;
}

DescriptionIndex Tribes::safe_immovable_index(const std::string& immovablename) const {
	const DescriptionIndex result = immovable_index(immovablename);
	if (!immovable_exists(result)) {
		throw GameDataError("Unknown immovable type \"%s\"", immovablename.c_str());
	}
	return result;
}

DescriptionIndex Tribes::safe_ship_index(const std::string& shipname) const {
	const DescriptionIndex result = ship_index(shipname);
	if (!ship_exists(result)) {
		throw GameDataError("Unknown ship type \"%s\"", shipname.c_str());
	}
	return result;
}

DescriptionIndex Tribes::safe_tribe_index(const std::string& tribename) const {
	const DescriptionIndex result = tribe_index(tribename);
	if (!tribe_exists(result)) {
		throw GameDataError("Unknown tribe \"%s\"", tribename.c_str());
	}
	return result;
}

DescriptionIndex Tribes::safe_ware_index(const std::string& warename) const {
	const DescriptionIndex result = ware_index(warename);
	if (!ware_exists(result)) {
		throw GameDataError("Unknown ware type \"%s\"", warename.c_str());
	}
	return result;
}

DescriptionIndex Tribes::safe_worker_index(const std::string& workername) const {
	const DescriptionIndex result = worker_index(workername);
	if (!worker_exists(result)) {
		throw GameDataError("Unknown worker type \"%s\"", workername.c_str());
	}
	return result;
}

DescriptionIndex Tribes::building_index(const std::string& buildingname) const {
	return buildings_->get_index(buildingname);
}

DescriptionIndex Tribes::immovable_index(const std::string& immovablename) const {
	return immovables_->get_index(immovablename);
}

DescriptionIndex Tribes::ship_index(const std::string& shipname) const {
	return ships_->get_index(shipname);
}

DescriptionIndex Tribes::tribe_index(const std::string& tribename) const {
	return tribes_->get_index(tribename);
}

DescriptionIndex Tribes::ware_index(const std::string& warename) const {
	return wares_->get_index(warename);
}

DescriptionIndex Tribes::worker_index(const std::string& workername) const {
	return workers_->get_index(workername);
}

const BuildingDescr* Tribes::get_building_descr(DescriptionIndex buildingindex) const {
	return buildings_->get_mutable(buildingindex);
}

BuildingDescr* Tribes::get_mutable_building_descr(DescriptionIndex buildingindex) const {
	return buildings_->get_mutable(buildingindex);
}

const ImmovableDescr* Tribes::get_immovable_descr(DescriptionIndex immovableindex) const {
	return immovables_->get_mutable(immovableindex);
}

const ShipDescr* Tribes::get_ship_descr(DescriptionIndex shipindex) const {
	return ships_->get_mutable(shipindex);
}

const WareDescr* Tribes::get_ware_descr(DescriptionIndex wareindex) const {
	return wares_->get_mutable(wareindex);
}

const WorkerDescr* Tribes::get_worker_descr(DescriptionIndex workerindex) const {
	return workers_->get_mutable(workerindex);
}

const TribeDescr* Tribes::get_tribe_descr(DescriptionIndex tribeindex) const {
	return tribes_->get_mutable(tribeindex);
}

void Tribes::set_ware_type_has_demand_check(DescriptionIndex wareindex,
                                            const std::string& tribename) const {
	wares_->get_mutable(wareindex)->set_has_demand_check(tribename);
}

void Tribes::set_worker_type_has_demand_check(DescriptionIndex workerindex) const {
	workers_->get_mutable(workerindex)->set_has_demand_check();
}

// ************************ Loading *************************

void Tribes::register_object(const LuaTable& table) {
    const std::string name(table.get_string("name"));
    const std::string script_path(table.get_string("script"));
    if (registered_tribe_objects_.count(name) == 1) {
        throw GameDataError("Tribes::register_object: Attempt to register object\n   name: '%s'\n   script: '%s'\nbut the object already exists", name.c_str(), script_path.c_str());
    }
    if (!g_fs->file_exists(script_path)) {
        throw GameDataError("Tribes::register_object: Attempt to register object\n   name: '%s'\n   script: '%s'\nbut the script file does not exist", name.c_str(), script_path.c_str());
    }
    registered_tribe_objects_.insert(std::make_pair(name, script_path));
}

void Tribes::add_tribe_object_type(const LuaTable& table, EditorGameBase& egbase, MapObjectType type) {
    const std::string object_name(table.get_string("name"));
    const std::string msgctxt(table.get_string("msgctxt"));
    const std::string object_descname = pgettext_expr(msgctxt.c_str(), table.get_string("descname").c_str());

    // Register as in progress
    tribe_objects_being_loaded_.insert(object_name);

    // Add
    i18n::Textdomain td("tribes");
    switch (type) {
    case MapObjectType::CARRIER:
        workers_->add(new CarrierDescr(object_descname, table, egbase));
        break;
    case MapObjectType::CONSTRUCTIONSITE:
        buildings_->add(new ConstructionSiteDescr(object_descname, table, egbase));
        break;
    case MapObjectType::DISMANTLESITE:
        buildings_->add(new DismantleSiteDescr(object_descname, table, egbase));
        break;
    case MapObjectType::IMMOVABLE:
        immovables_->add(new ImmovableDescr(object_descname, table, *this));
        break;
    case MapObjectType::MARKET:
        buildings_->add(new MarketDescr(object_descname, table, egbase));
        break;
    case MapObjectType::MILITARYSITE:
        buildings_->add(new MilitarySiteDescr(object_descname, table, egbase));
        break;
    case MapObjectType::PRODUCTIONSITE:
        buildings_->add(
           new ProductionSiteDescr(object_descname, msgctxt, table, egbase));
        break;
    case MapObjectType::SHIP:
        ships_->add(new ShipDescr(object_descname, table));
        break;
    case MapObjectType::SOLDIER:
        workers_->add(new SoldierDescr(object_descname, table, egbase));
        break;
    case MapObjectType::TRAININGSITE:
        buildings_->add(
           new TrainingSiteDescr(object_descname, msgctxt, table, egbase));
        break;
    case MapObjectType::WARE:
        wares_->add(new WareDescr(object_descname, table));
        break;
    case MapObjectType::WAREHOUSE:
        buildings_->add(new WarehouseDescr(object_descname, table, egbase));
        break;
    case MapObjectType::WORKER:
        workers_->add(new WorkerDescr(object_descname, table, egbase));
        break;
    default:
        NEVER_HERE();
    }

    // Update status
    loaded_tribe_objects_.insert(object_name);

    // Mark as done
    tribe_objects_being_loaded_.erase(tribe_objects_being_loaded_.find(object_name));
}

void Tribes::add_tribe(const LuaTable& table) {
	const std::string name = table.get_string("name");
	if (Widelands::tribe_exists(name)) {
		tribes_->add(new TribeDescr(table, Widelands::get_tribeinfo(name), *this));
	} else {
		throw GameDataError("The tribe '%s'' has no preload file.", name.c_str());
	}
}

void Tribes::add_custom_building(const LuaTable& table) {
    // NOCOM decide what to do with these
	const std::string tribename = table.get_string("tribename");
	if (Widelands::tribe_exists(tribename)) {
		TribeDescr* descr = tribes_->get_mutable(tribe_index(tribename));
        const std::string object_name(table.get_string("buildingname"));
		descr->add_building(object_name, *this);
	} else {
		throw GameDataError("The tribe '%s'' has no preload file.", tribename.c_str());
	}
}

void Tribes::load_object(const std::string& object_name) {
    // Nothing to do if it's already loaded
    if (loaded_tribe_objects_.count(object_name) == 1) {
        return;
    }

    // Make sure that we have it
    if (registered_tribe_objects_.count(object_name) != 1) {
        throw GameDataError("Tribes::load_object: Object '%s' was not registered", object_name.c_str());
    }

    // Protect against circular dependencies
    if (tribe_objects_being_loaded_.count(object_name) == 1) {
        throw GameDataError("Tribes::load_object: Object '%s' is already being loaded", object_name.c_str());
    }

    // Load it
    lua_->run_script(registered_tribe_objects_.at(object_name));
}

DescriptionIndex Tribes::load_building(const std::string& buildingname) {
    try {
        load_object(buildingname);
    } catch (WException& e) {
        throw GameDataError("Error while loading building type '%s': %s", buildingname.c_str(), e.what());
    }
    return safe_building_index(buildingname);
}

DescriptionIndex Tribes::load_immovable(const std::string& immovablename) {
    try {
        load_object(immovablename);
    } catch (WException& e) {
        throw GameDataError("Error while loading immovable type '%s': %s", immovablename.c_str(), e.what());
    }
    return safe_immovable_index(immovablename);
}

DescriptionIndex Tribes::load_ship(const std::string& shipname) {
    try {
        load_object(shipname);
    } catch (WException& e) {
        throw GameDataError("Error while loading ship type '%s': %s", shipname.c_str(), e.what());
    }
    return safe_ship_index(shipname);
}

DescriptionIndex Tribes::load_ware(const std::string& warename) {
    try {
        load_object(warename);
    } catch (WException& e) {
        throw GameDataError("Error while loading ware type '%s': %s", warename.c_str(), e.what());
    }
    return safe_ware_index(warename);
}

DescriptionIndex Tribes::load_worker(const std::string& workername) {
    try {
        load_object(workername);
    } catch (WException& e) {
        throw GameDataError("Error while loading worker type '%s': %s", workername.c_str(), e.what());
    }
    return safe_worker_index(workername);
}

void Tribes::load_graphics() {
	for (size_t tribeindex = 0; tribeindex < nrtribes(); ++tribeindex) {
		TribeDescr* tribe = tribes_->get_mutable(tribeindex);
		for (const std::string& texture_path : tribe->normal_road_paths()) {
			tribe->add_normal_road_texture(g_gr->images().get(texture_path));
		}
		for (const std::string& texture_path : tribe->busy_road_paths()) {
			tribe->add_busy_road_texture(g_gr->images().get(texture_path));
		}
	}
}

void Tribes::postload() {
    // NOCOM move this to TribeDescr. A bit less efficient, but needed for flexibility
	for (DescriptionIndex i = 0; i < buildings_->size(); ++i) {
		BuildingDescr& building_descr = *buildings_->get_mutable(i);

		// Add consumers and producers to wares.
		if (upcast(ProductionSiteDescr, de, &building_descr)) {
			for (const auto& ware_amount : de->input_wares()) {
				wares_->get_mutable(ware_amount.first)->add_consumer(i);
			}
			for (const DescriptionIndex& wareindex : de->output_ware_types()) {
				wares_->get_mutable(wareindex)->add_producer(i);
			}
			for (const auto& job : de->working_positions()) {
				workers_->get_mutable(job.first)->add_employer(i);
			}
		}

		// Register which buildings buildings can have been enhanced from
		const DescriptionIndex& enhancement = building_descr.enhancement();
		if (building_exists(enhancement)) {
			buildings_->get_mutable(enhancement)->set_enhanced_from(i);
		}
	}

	// Calculate the trainingsites proportions.
	postload_calculate_trainingsites_proportions();

	// Resize the configuration of our wares if they won't fit in the current window (12 = info label
	// size).
	// Also, do some final checks on the gamedata
	int number = (g_gr->get_yres() - 290) / (WARE_MENU_PIC_HEIGHT + WARE_MENU_PIC_PAD_Y + 12);
	for (DescriptionIndex i = 0; i < tribes_->size(); ++i) {
		TribeDescr* tribe_descr = tribes_->get_mutable(i);
		tribe_descr->resize_ware_orders(number);

		// Verify that the preciousness has been set for all of the tribe's wares
		for (const DescriptionIndex wi : tribe_descr->wares()) {
			if (tribe_descr->get_ware_descr(wi)->preciousness(tribe_descr->name()) == kInvalidWare) {
				throw GameDataError("The ware '%s' needs to define a preciousness for tribe '%s'",
				                    tribe_descr->get_ware_descr(wi)->name().c_str(),
				                    tribe_descr->name().c_str());
			}
		}
	}
}

// Set default trainingsites proportions for AI. Make sure that we get a sum of ca. 100
void Tribes::postload_calculate_trainingsites_proportions() {
	for (DescriptionIndex i = 0; i < tribes_->size(); ++i) {
		TribeDescr* tribe_descr = tribes_->get_mutable(i);
		unsigned int trainingsites_without_percent = 0;
		int used_percent = 0;
		std::vector<BuildingDescr*> traingsites_with_percent;
		for (const DescriptionIndex& index : tribe_descr->trainingsites()) {
			BuildingDescr* descr = get_mutable_building_descr(index);
			if (descr->hints().trainingsites_max_percent() == 0) {
				++trainingsites_without_percent;
			} else {
				used_percent += descr->hints().trainingsites_max_percent();
				traingsites_with_percent.push_back(descr);
			}
		}

		log("%s trainingsites: We have used up %d%% on %" PRIuS " sites, there are %d without\n",
		    tribe_descr->name().c_str(), used_percent, traingsites_with_percent.size(),
		    trainingsites_without_percent);

		// Adjust used_percent if we don't have at least 5% for each remaining trainingsite
		const float limit = 100 - trainingsites_without_percent * 5;
		if (used_percent > limit) {
			const int deductme = (used_percent - limit) / traingsites_with_percent.size();
			used_percent = 0;
			for (BuildingDescr* descr : traingsites_with_percent) {
				descr->set_hints_trainingsites_max_percent(descr->hints().trainingsites_max_percent() -
				                                           deductme);
				used_percent += descr->hints().trainingsites_max_percent();
			}
			log("%s trainingsites: Used percent was adjusted to %d%%\n", tribe_descr->name().c_str(),
			    used_percent);
		}

		// Now adjust for trainingsites that didn't have their max_percent set
		if (trainingsites_without_percent > 0) {
			int percent_to_use = std::ceil((100 - used_percent) / trainingsites_without_percent);
			// We sometimes get below 100% in spite of the ceil call above.
			// A total sum a bit above 100% is fine though, so we increment until it's big enough.
			while ((used_percent + percent_to_use * trainingsites_without_percent) < 100) {
				++percent_to_use;
			}
			log("%s trainingsites: Assigning %d%% to each of the remaining %d sites\n",
			    tribe_descr->name().c_str(), percent_to_use, trainingsites_without_percent);
			if (percent_to_use < 1) {
				throw GameDataError(
				   "%s: Training sites without predefined proportions add up to < 1%% and "
				   "will never be built: %d",
				   tribe_descr->name().c_str(), used_percent);
			}
			for (const DescriptionIndex& index : tribe_descr->trainingsites()) {
				BuildingDescr* descr = get_mutable_building_descr(index);
				if (descr->hints().trainingsites_max_percent() == 0) {
					descr->set_hints_trainingsites_max_percent(percent_to_use);
					used_percent += percent_to_use;
				}
			}
		}
		if (used_percent < 100) {
			throw GameDataError("%s: Final training sites proportions add up to < 100%%: %d",
			                    tribe_descr->name().c_str(), used_percent);
		}
	}
}

}  // namespace Widelands
