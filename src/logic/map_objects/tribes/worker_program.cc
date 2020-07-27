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

#include "logic/map_objects/tribes/worker_program.h"

#include "base/log.h"
#include "logic/game_data_error.h"
#include "logic/map_objects/findnode.h"
#include "sound/sound_handler.h"

namespace Widelands {
/* RST
.. _tribes_worker_programs:

Worker Programs
===============

Worker programs are defined in the ``programs`` subtable specified in the worker's
:ref:`lua_tribes_workers_common`. Each worker program is a Lua table in itself and defined as a
series of command strings. Commands can also have parameters, which are separated from each other by
a blank space. These parameters can also have values, which are separated from the parameter name by
a colon (:). Finally, programs can call other programs. The table looks like this::

   programs = {
      program_name1 = {
         "program_name2",
         "program_name3",
      }
      program_name2 = {
         "command1=parameter1:value1 parameter2:value2",
         "command2=parameter1",
      },
      program_name3 = {
         "command3",
         "command4=parameter1 parameter2 parameter3",
      }
   },

The available commands are:

- `createware`_
- `mine`_
- `breed`_
- `findobject`_
- `findspace`_
- `walk`_
- `animate`_
- `return`_
- `callobject`_
- `plant`_
- `createbob`_
- `buildferry`_
- `removeobject`_
- `repeatsearch`_
- `findresources`_
- `scout`_
- `playsound`_
- `construct`_
- `terraform`_
*/

const WorkerProgram::ParseMap WorkerProgram::parsemap_[] = {
   {"mine", &WorkerProgram::parse_mine},
   {"breed", &WorkerProgram::parse_breed},
   {"createware", &WorkerProgram::parse_createware},
   {"findobject", &WorkerProgram::parse_findobject},
   {"findspace", &WorkerProgram::parse_findspace},
   {"walk", &WorkerProgram::parse_walk},
   {"animate", &WorkerProgram::parse_animate},
   {"return", &WorkerProgram::parse_return},
   {"callobject", &WorkerProgram::parse_callobject},
   {"plant", &WorkerProgram::parse_plant},
   {"createbob", &WorkerProgram::parse_createbob},
   {"buildferry", &WorkerProgram::parse_buildferry},
   {"removeobject", &WorkerProgram::parse_removeobject},
   {"repeatsearch", &WorkerProgram::parse_repeatsearch},
   {"findresources", &WorkerProgram::parse_findresources},
   {"scout", &WorkerProgram::parse_scout},
   {"playsound", &WorkerProgram::parse_playsound},
   {"construct", &WorkerProgram::parse_construct},
   {"terraform", &WorkerProgram::parse_terraform},

   {nullptr, nullptr}};

/**
 * Parse a program
 */
WorkerProgram::WorkerProgram(const std::string& init_name,
                             const LuaTable& actions_table,
                             const WorkerDescr& worker,
                             Tribes& tribes)
   : MapObjectProgram(init_name), worker_(worker), tribes_(tribes) {

	for (const std::string& line : actions_table.array_entries<std::string>()) {
		if (line.empty()) {
			throw GameDataError("Empty line");
		}
		try {

			ProgramParseInput parseinput = parse_program_string(line);

			// Find the appropriate parser
			Worker::Action act;
			uint32_t mapidx;

			for (mapidx = 0; parsemap_[mapidx].name; ++mapidx) {
				if (parseinput.name == parsemap_[mapidx].name) {
					break;
				}
			}

			if (!parsemap_[mapidx].name) {
				throw GameDataError(
				   "Unknown command '%s' in line '%s'", parseinput.name.c_str(), line.c_str());
			}

			(this->*parsemap_[mapidx].function)(&act, parseinput.arguments);

			actions_.push_back(act);
		} catch (const std::exception& e) {
			throw GameDataError("Error reading line '%s': %s", line.c_str(), e.what());
		}
	}
	if (actions_.empty()) {
		throw GameDataError("No actions found");
	}
}

/* RST
createware
^^^^^^^^^^
.. function:: createware=\<ware_name\>

   :arg string ware_name: The ware type to create, e.g. ``wheat``.

   The worker will create and carry a ware of the given type. Example::

      harvest = {
         "findobject=attrib:ripe_wheat radius:2",
         "walk=object",
         "playsound=sound/farm/scythe 220",
         "animate=harvesting 10000",
         "callobject=harvest",
         "animate=gathering 4000",
         "createware=wheat", -- Create 1 wheat and start carrying it
         "return"
      },
*/
/**
 * iparam1 = ware index
 */
void WorkerProgram::parse_createware(Worker::Action* act, const std::vector<std::string>& cmd) {
	if (cmd.size() != 1) {
		throw wexception("Usage: createware=<ware type>");
	}

	const DescriptionIndex ware_index = tribes_.load_ware(cmd[0]);

	act->function = &Worker::run_createware;
	act->iparam1 = ware_index;
	produced_ware_types_.insert(ware_index);
}

/* RST
mine
^^^^
.. function:: mine=\<resource_name\> \<area\>

   :arg string resource_name: The map resource to mine, e.g. ``fish``.

   :arg int area: The radius that is scanned for decreasing the map resource, e.g. ``1``.

   Mine on the current coordinates that the worker has walked to for resources decrease.
   Example::

      fish = {
         "findspace=size:any radius:7 resource:fish",
         "walk=coords",
         "playsound=sound/fisher/fisher_throw_net 192",
         "mine=fish 1", -- Remove a fish in an area of 1
         "animate=fishing 3000",
         "playsound=sound/fisher/fisher_pull_net 192",
         "createware=fish",
         "return"
      },
*/
/**
 * iparam1 = area
 * sparam1 = resource
 */
void WorkerProgram::parse_mine(Worker::Action* act, const std::vector<std::string>& cmd) {
	if (cmd.size() != 2) {
		throw GameDataError("Usage: mine=<ware type> <workarea radius>");
	}

	act->function = &Worker::run_mine;
	act->sparam1 = cmd[0];
	act->iparam1 = read_positive(cmd[1]);
}

/* RST
breed
^^^^^
.. function:: breed=\<resource_name\> \<area\>

   :arg string resource_name: The map resource to breed, e.g. ``fish``.

   :arg int area: The radius that is scanned for increasing the map resource, e.g. ``1``.

   Breed a resource on the current coordinates that the worker has walked to for
   resources increase. Example::

      breed = {
         "findspace=size:any radius:7 breed resource:fish",
         "walk=coords",
         "animate=freeing 3000",
         "breed=fish 1", -- Add a fish in an area of 1
         "return"
      },
*/
/**
 * iparam1 = area
 * sparam1 = resource
 */
void WorkerProgram::parse_breed(Worker::Action* act, const std::vector<std::string>& cmd) {
	if (cmd.size() != 2) {
		throw GameDataError("Usage: breed=<ware type> <workarea radius>");
	}

	act->function = &Worker::run_breed;
	act->sparam1 = cmd[0];
	act->iparam1 = read_positive(cmd[1]);
}

/* RST
findobject
^^^^^^^^^^
.. function:: findobject=radius:\<distance\> [type:\<map_object_type\>] [attrib:\<attribute\>]

   :arg int radius: Search for an object within the given radius around the worker.
   :arg string type: The type of map object to search for. Defaults to ``immovable``.
   :arg string attrib: The attribute that the map object should possess.

   Find and select an object based on a number of predicates, which can be specified
   in arbitrary order. The object can then be used in other commands like ``walk``
   or ``callobject``. Examples::

      cut_granite = {
         "findobject=attrib:rocks radius:6", -- Find rocks on the map within a radius of 6 from your
         building
         "walk=object", -- Now walk to those rocks
         "playsound=sound/atlanteans/cutting/stonecutter 192",
         "animate=hacking 12000",
         "callobject=shrink",
         "createware=granite",
         "return"
      },

      hunt = {
         "findobject=type:bob radius:13 attrib:eatable", -- Find an eatable bob (animal) within a
         radius of 13 from your building
         "walk=object", -- Walk to where the animal is
         "animate=idle 1500",
         "removeobject",
         "createware=meat",
         "return"
      },
*/
/**
 * iparam1 = radius predicate
 * iparam2 = attribute predicate (if >= 0)
 * sparam1 = type
 */
void WorkerProgram::parse_findobject(Worker::Action* act, const std::vector<std::string>& cmd) {
	act->function = &Worker::run_findobject;
	act->iparam1 = -1;
	act->iparam2 = -1;
	act->sparam1 = "immovable";

	// Parse predicates
	for (const std::string& argument : cmd) {
		const std::pair<std::string, std::string> item = read_key_value_pair(argument, ':');

		if (item.first == "radius") {
			act->iparam1 = read_positive(item.second);
		} else if (item.first == "attrib") {
			act->iparam2 = MapObjectDescr::get_attribute_id(item.second);
		} else if (item.first == "type") {
			act->sparam1 = item.second;
		} else {
			throw GameDataError("Unknown findobject predicate %s", argument.c_str());
		}
	}

	workarea_info_[act->iparam1].insert(" findobject");
}

/* RST
findspace
^^^^^^^^^
.. function:: findspace=size:\<plot\> radius:\<distance\> [breed] [resource:\<name\>]
   [avoid:\<immovable_attribute\>] [saplingsearches:\<number\>] [space] [terraform]

   :arg string size: The size or building plot type of the free space.
      The possible values are:

      * ``any``: Any size will do.
      * ``build``: Any building plot.
      * ``small``: Small building plots only.
      * ``medium``: Medium building plots only.
      * ``big``: Big building plots only.
      * ``mine``: Mining plots only.
      * ``port``: Port spaces only.
      * ``swim``: Anything on the coast.

   :arg int radius: Search for map fields within the given radius around the worker.

   :arg empty breed: Used in front of ``resource`` only: Also accept fields where the
      resource has been depleted. Use this when looking for a place for breeding.

   :arg string resource: A resource to search for. This is mainly intended for
      fishers and suchlike, for non-detectable resources and default resources.

   :arg string avoid: A field containing an immovable that has this attribute will
      not be used.

   :arg int saplingsearches: The higher the number, the better the accuracy
      for finding a better spot for immovables that have terrain affinity, e.g. trees.

   :arg empty space: Find only fields that are walkable in such a way that all
      neighbors are also walkable (an exception is made if one of the neighboring
      fields is owned by this worker's location).

   :arg empty terraform: Find only nodes where at least one adjacent triangle has
      terrain that can be enhanced

   Find a map field based on a number of predicates.
   The field can then be used in other commands like ``walk``. Examples::

      breed = {
         -- Find any field that can have fish in it for adding a fish to it below
         "findspace=size:any radius:7 breed resource:fish",
         "walk=coords",
         "animate=freeing 3000",
         "breed=fish 1",
         "return"
      },

      plant = {
         -- Don't get in the way of the farmer's crops when planting trees. Retry 8 times.
         "findspace=size:any radius:5 avoid:field saplingsearches:8",
         "walk=coords",
         "animate=dig 2000",
         "animate=planting 1000",
         "plant=attrib:tree_sapling",
         "animate=water 2000",
         "return"
      },

      plant = {
         -- The farmer will want to walk to this field again later for harvesting his crop
         "findspace=size:any radius:2 space",
         "walk=coords",
         "animate=planting 4000",
         "plant=attrib:seed_wheat",
         "animate=planting 4000",
         "return",
      },
*/
/**
 * iparam1 = radius
 * iparam2 = FindNodeSize::sizeXXX
 * iparam3 = whether the "space" flag is set
 * iparam4 = whether the "breed" flag is set
 * iparam5 = Immovable attribute id
 * iparam6 = Forester retries
 * iparam7 = whether the "terraform" flag is set
 * sparam1 = Resource
 */
void WorkerProgram::parse_findspace(Worker::Action* act, const std::vector<std::string>& cmd) {
	act->function = &Worker::run_findspace;
	act->iparam1 = -1;
	act->iparam2 = -1;
	act->iparam3 = 0;
	act->iparam4 = 0;
	act->iparam5 = -1;
	act->iparam6 = 1;
	act->iparam7 = 0;
	act->sparam1 = "";

	// Parse predicates
	for (const std::string& argument : cmd) {
		try {
			const std::pair<std::string, std::string> item = read_key_value_pair(argument, ':');

			if (item.first == "radius") {
				act->iparam1 = read_positive(item.second);
			} else if (item.first == "size") {
				static const std::map<std::string, FindNodeSize::Size> sizenames{
				   {"any", FindNodeSize::sizeAny},     {"build", FindNodeSize::sizeBuild},
				   {"small", FindNodeSize::sizeSmall}, {"medium", FindNodeSize::sizeMedium},
				   {"big", FindNodeSize::sizeBig},     {"mine", FindNodeSize::sizeMine},
				   {"port", FindNodeSize::sizePort},   {"swim", FindNodeSize::sizeSwim}};

				if (sizenames.count(item.second) != 1) {
					throw GameDataError("Bad findspace size '%s'", item.second.c_str());
				}
				act->iparam2 = sizenames.at(item.second);
			} else if (item.first == "breed") {
				act->iparam4 = 1;
			} else if (item.first == "terraform") {
				act->iparam7 = 1;
			} else if (item.first == "resource") {
				act->sparam1 = item.second;
			} else if (item.first == "space") {
				act->iparam3 = 1;
			} else if (item.first == "avoid") {
				act->iparam5 = MapObjectDescr::get_attribute_id(item.second);
			} else if (item.first == "saplingsearches") {
				act->iparam6 = read_int(item.second, 2);
			} else {
				throw GameDataError("Unknown findspace predicate %s", item.first.c_str());
			}
		} catch (const GameDataError& e) {
			throw GameDataError("Malformed findspace argument %s: %s", argument.c_str(), e.what());
		}
	}

	if (act->iparam1 <= 0) {
		throw GameDataError("findspace: must specify radius");
	}
	if (act->iparam2 < 0) {
		throw GameDataError("findspace: must specify size");
	}
	workarea_info_[act->iparam1].insert(" findspace");
}

/* RST
walk
^^^^
.. function:: walk=\<destination_type\>

   :arg string destination_type: Defines where to walk to. Possible destinations are:

      * ``object``: Walk to a previously found and selected object.
      * ``coords``: Walk to a previously found and selected field/coordinate.
      * ``object-or-coords``: Walk to a previously found and selected object if present;
        otherwise to previously found and selected field/coordinate.

   Walk to a previously selected destination. Examples::

      plant = {
         "findspace=size:any radius:2",
         "walk=coords", -- Walk to the space found by the command above
         "animate=planting 4000",
         "plant=attrib:seed_blackroot",
         "animate=planting 4000",
         "return"
      },

      harvest = {
         "findobject=attrib:ripe_blackroot radius:2",
         "walk object", -- Walk to the blackroot field found by the command above
         "animate=harvesting 10000",
         "callobject=harvest",
         "animate=gathering 2000",
         "createware=blackroot",
         "return"
      },

      buildship = {
         "walk=object-or-coords", -- Walk to coordinates from 1. or to object from 2.
         -- 2. This will create an object for us if we don't have one yet
         "plant=attrib:shipconstruction unless object",
         "playsound=sound/sawmill/sawmill 230",
         "animate=work 500",
         "construct", -- 1. This will find a space for us if no object has been planted yet
         "animate=work 5000",
         "return"
      },
*/
/**
 * iparam1 = walkXXX
 */
void WorkerProgram::parse_walk(Worker::Action* act, const std::vector<std::string>& cmd) {
	if (cmd.size() != 1) {
		throw GameDataError("Usage: walk=object|coords|object-or-coords");
	}

	act->function = &Worker::run_walk;

	if (cmd[0] == "object") {
		act->iparam1 = Worker::Action::walkObject;
	} else if (cmd[0] == "coords") {
		act->iparam1 = Worker::Action::walkCoords;
	} else if (cmd[0] == "object-or-coords") {
		act->iparam1 = Worker::Action::walkObject | Worker::Action::walkCoords;
	} else {
		throw GameDataError("Bad walk destination '%s'", cmd[0].c_str());
	}
}

/* RST
animate
^^^^^^^
.. function:: animate=\<name\> \<duration\>

   :arg string name: The name of the animation.
   :arg int duration: The time in milliseconds for which the animation will be played.

   Play the given animation for the given duration. Example::

      plantvine = {
         "findspace=size:any radius:1",
         "walk=coords",
         "animate=dig 2000", -- Play a digging animation for 2 seconds.
         "plant=attrib:seed_grapes",
         "animate=planting 3000", -- Play a planting animation for 3 seconds.
         "return"
      },
*/
/**
 * iparam1 = anim id
 * iparam2 = duration
 */
void WorkerProgram::parse_animate(Worker::Action* act, const std::vector<std::string>& cmd) {
	AnimationParameters parameters = MapObjectProgram::parse_act_animate(cmd, worker_, true);

	act->function = &Worker::run_animate;
	// If the second parameter to MapObjectDescr::get_animation is ever used for anything other than
	// level-dependent soldier animations, or we want to write a worker program for a soldier,
	// we will need to store the animation name as a string in an sparam
	act->iparam1 = parameters.animation;
	act->iparam2 = parameters.duration;
}

/* RST
return
^^^^^^
.. function:: return

   Return home and then drop any ware we're carrying onto our building's flag. Example::

      scout = {
         "scout=15 75000",
         "return" -- Go home
      }
*/
/**
 * iparam1 = 0: don't drop ware on flag, 1: do drop ware on flag
 */
void WorkerProgram::parse_return(Worker::Action* act, const std::vector<std::string>& cmd) {
	if (!cmd.empty()) {
		throw GameDataError("Usage: return");
	}
	act->function = &Worker::run_return;
	act->iparam1 = 1;  // drop a ware on our owner's flag
}

/* RST
callobject
^^^^^^^^^^^
.. function:: callobject=\<program_name\>

   :arg string program_name: The name of the program to be executed.

   Cause the currently selected object to execute its given program. Example::

      harvest = {
         "findobject=attrib:tree radius:10",
         "walk=object",
         "playsound=sound/woodcutting/fast_woodcutting 250",
         "animate=hacking 10000",
         "playsound=sound/woodcutting/tree-falling 130",
         "callobject=fall", -- Cause the tree to fall
         "animate=idle 2000",
         "createware=log",
         "return"
      }

   See also :doc:`immovable_program`.
*/
/**
 * sparam1 = callobject command name
 */
void WorkerProgram::parse_callobject(Worker::Action* act, const std::vector<std::string>& cmd) {
	if (cmd.size() != 1) {
		throw GameDataError("Usage: callobject=<program name>");
	}

	act->function = &Worker::run_callobject;
	act->sparam1 = cmd[0];
}

/* RST
plant
^^^^^
.. function:: plant attrib:\<attribute\> [attrib:\<attribute\> ...] [unless object]

   :arg string attrib\:\<attribute\>: Select at random any world immovable or immovable
      of the worker's tribe that has this attribute.

   :arg empty unless object: Do not plant the immovable if it already exists at
      the current position.

   Plant one of the given immovables on the current position, taking into account
   the fertility of the area. Examples::

      plant = {
         "findspace=size:any radius:5 avoid:field",
         "walk=coords",
         "animate=dig 2000",
         "animate=planting 1000",
         "plant=attrib:tree_sapling", -- Plant any random sapling tree
         "animate=water 2000",
         "return"
      },

      plant = {
         "findspace=size:any radius:2 space",
         "walk=coords",
         "animate=planting 4000",
         -- Plant the tiny field immovable that the worker's tribe knows about
         "plant=attrib:seed_wheat",
         "animate=planting 4000",
         "return",
      },

      buildship = {
         "walk=object-or-coords",
         -- Only create a shipconstruction if we don't already have one
         "plant=attrib:shipconstruction unless object",
         "playsound=sound/sawmill/sawmill 230",
         "animate=work 500",
         "construct",
         "animate=work 5000",
         "return"
      }
*/
/**
 * sparamv  list of attributes
 * iparam1  one of plantXXX
 */
void WorkerProgram::parse_plant(Worker::Action* act, const std::vector<std::string>& cmd) {
	if (cmd.empty()) {
		throw GameDataError(
		   "Usage: plant=attrib:<attribute> [attrib:<attribute> ...] [unless object]");
	}

	act->function = &Worker::run_plant;
	act->iparam1 = Worker::Action::plantAlways;
	for (uint32_t i = 0; i < cmd.size(); ++i) {
		if (cmd[i] == "unless") {
			++i;
			if (i >= cmd.size()) {
				throw GameDataError("plant: something expected after 'unless'");
			}
			if (cmd[i] == "object") {
				act->iparam1 = Worker::Action::plantUnlessObject;
			} else {
				throw GameDataError("plant: 'unless %s' not understood", cmd[i].c_str());
			}
			continue;
		}

		const std::string attrib_name = read_key_value_pair(cmd[i], ':', "", "attrib").second;

		// This will throw a GameDataError if the attribute doesn't exist.
		ImmovableDescr::get_attribute_id(attrib_name);
		act->sparamv.push_back(attrib_name);
	}
}

/* RST
createbob
^^^^^^^^^
.. function:: createbob=\<bob_name\> [\<bob_name\> ...]

   :arg string bob_name: The bob type to add to the selection. Specify as many bob
      types as you want.

   Adds a bob (an animal or a worker, e.g. a deer or a ferry) to the map at the worker's current
   location. Randomly select from the list of ``bob_name``. Examples::

      release = {
         "findspace=size:any radius:3",
         "walk=coords",
         "animate=releasein 2000",
         "createbob=wildboar stag sheep", -- Release a wildboar, stag or sheep into the wild
         "animate=releaseout 2000",
         "return"
      },

      buildferry = {
         "findspace=size:swim radius:5",
         "walk=coords",
         "animate=work 10000",
         "createbob=frisians_ferry",
         "return"
      }
*/
// TODO(GunChleoc): attrib:eatable would be much better, then depend on terrain too
void WorkerProgram::parse_createbob(Worker::Action* act, const std::vector<std::string>& cmd) {
	if (cmd.empty()) {
		throw GameDataError("Usage: createbob=<bob name> <bob name> ...");
	}

	act->function = &Worker::run_createbob;
	act->sparamv = std::move(cmd);
}

/* RST
buildferry
^^^^^^^^^^
.. function:: buildferry

   **DEPRECATED** use ``createbob=TRIBENAME_ferry`` instead.
*/
void WorkerProgram::parse_buildferry(Worker::Action* act, const std::vector<std::string>& cmd) {
	if (cmd.size() > 1) {
		throw wexception("buildferry takes no arguments");
	}
	act->function = &Worker::run_buildferry;
}

/* RST
terraform
^^^^^^^^^
.. function:: terraform

   Turns the terrain of one of the triangles around the current node into its
   enhancement terrain. Example::

      terraform = {
         "findspace=size:terraform radius:6",
         "walk=coords",
         "animate=dig 2000",
         "terraform",
         "return"
      }
*/
void WorkerProgram::parse_terraform(Worker::Action* act, const std::vector<std::string>& cmd) {
	if (cmd.size() > 1) {
		throw wexception("terraform takes no arguments");
	}
	act->function = &Worker::run_terraform;
}

/* RST
removeobject
^^^^^^^^^^^^
.. function:: removeobject

   Remove the currently selected object. Example::

      hunt = {
         "findobject=type:bob radius:13 attrib:eatable", -- Select an object to remove
         "walk=object",
         "animate=idle 1000",
         -- The selected eatable map object has been hunted, so remove it from the map
         "removeobject",
         "createware=meat",
         "return"
      }
*/
void WorkerProgram::parse_removeobject(Worker::Action* act, const std::vector<std::string>&) {
	act->function = &Worker::run_removeobject;
}

/* RST
repeatsearch
^^^^^^^^^^^^
.. function:: repeatsearch=\<repetitions\> \<radius\> \<program_name\>

   :arg int repetitions: The number of times that the worker will move to a
      different spot on the map to execute ``program_name``. Used by geologists.

   :arg int radius: The radius of map fields for the worker not to stray from.

   Walk around the starting point randomly within a certain radius, and execute
   your ``program_name`` for some of the fields. Example::

      expedition = {
         "repeatsearch=15 5 search"
      },
*/
/**
 * iparam1 = maximum repeat #
 * iparam2 = radius
 * sparam1 = subcommand
 */
void WorkerProgram::parse_repeatsearch(Worker::Action* act, const std::vector<std::string>& cmd) {
	if (cmd.size() != 3) {
		throw GameDataError("Usage: repeatsearch=<repeat #> <radius> <subcommand>");
	}

	act->function = &Worker::run_repeatsearch;
	act->iparam1 = read_positive(cmd[0]);
	act->iparam2 = read_positive(cmd[1]);
	act->sparam1 = cmd[2];
}

/* RST
findresources
^^^^^^^^^^^^^
.. function:: findresources

   Check the current position for map resources (e.g. coal or water), and plant
   a marker object when possible. Example::

      search = {
         "animate=hacking 5000",
         "animate=idle 2000",
         "playsound=sound/hammering/geologist_hammer 192",
         "animate=hacking 3000",
         -- Plant a resource marker at the current location, according to what has been found.
         "findresources"
      }
*/
void WorkerProgram::parse_findresources(Worker::Action* act, const std::vector<std::string>& cmd) {
	if (!cmd.empty()) {
		throw GameDataError("Usage: findresources");
	}

	act->function = &Worker::run_findresources;
}

/* RST
scout
^^^^^
.. function:: scout=\<radius\> \<time\>

   :arg int radius: The radius of map fields for the scout to explore.

   :arg int time: The time in milliseconds that the scout will spend scouting.

   Sends a scout out to run around scouting the area. Example::

      scout = {
         "scout=15 75000", -- Scout within a radius of 15 for 75 seconds
         "return"
      },
*/
/**
 * iparam1 = radius
 * iparam2 = time
 */
void WorkerProgram::parse_scout(Worker::Action* act, const std::vector<std::string>& cmd) {
	if (cmd.size() != 2) {
		throw GameDataError("Usage: scout=<radius> <time>");
	}

	act->iparam1 = read_positive(cmd[0]);
	act->iparam2 = read_positive(cmd[1]);
	act->function = &Worker::run_scout;
}

/* RST
playsound
^^^^^^^^^^
.. function:: playsound=\<sound_dir/sound_name\> [priority]

   :arg string sound_dir/sound_name: The directory (folder) that the sound files are in,
      relative to the data directory, followed by the name of the particular sound to play.
      There can be multiple sound files to select from at random, e.g.
      for `sound/farm/scythe`, we can have `sound/farm/scythe_00.ogg`, `sound/farm/scythe_01.ogg`
      ...

   :arg int priority: The priority to give this sound. Maximum priority is 255.

   Play a sound effect. Example::

      harvest = {
         "findobject=attrib:ripe_wheat radius:2",
         "walk=object",
         "playsound=sound/farm/scythe 220", -- Almost certainly play a swishy harvesting sound
         "animate=harvesting 10000",
         "callobject=harvest",
         "animate=gathering 4000",
         "createware=wheat",
         "return"
      }
*/
void WorkerProgram::parse_playsound(Worker::Action* act, const std::vector<std::string>& cmd) {
	//  50% chance to play, only one instance at a time
	PlaySoundParameters parameters = MapObjectProgram::parse_act_play_sound(cmd, kFxPriorityMedium);

	act->iparam1 = parameters.priority;
	act->iparam2 = parameters.fx;
	act->function = &Worker::run_playsound;
}

/* RST
construct
^^^^^^^^^
.. function:: construct

   Give the ware currently held by the worker to the immovable object for construction.
   This is used in ship building. Example::

      buildship = {
         "walk=object-or-coords", -- Walk to coordinates from 1. or to object from 2.
         -- 2. This will create an object for us if we don't have one yet
         "plant=attrib:shipconstruction unless object",
         "playsound=sound/sawmill/sawmill 230",
         "animate=work 500",
         -- 1. Add the current ware to the shipconstruction. This will find a space for us if no
         -- shipconstruction object has been planted yet
         "construct",
         "animate=work 5000",
         "return"
      },
*/
/**
 * construct
 *
 * Give the currently held ware of the worker to the \ref objvar1 immovable
 * for construction. This is used in ship building.
 */
void WorkerProgram::parse_construct(Worker::Action* act, const std::vector<std::string>& cmd) {
	if (!cmd.empty()) {
		throw GameDataError("Usage: construct");
	}

	act->function = &Worker::run_construct;
}
}  // namespace Widelands
