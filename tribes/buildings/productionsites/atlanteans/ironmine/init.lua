dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   name = "atlanteans_ironmine",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Iron Mine",
   icon = dirname .. "menu.png",
   size = "mine",

   buildcost = {
		log = 7,
		planks = 4,
		spidercloth = 1
	},
	return_on_dismantle = {
		log = 2,
		planks = 2
	},

   helptexts = {
		-- TRANSLATORS: Lore helptext for a building
		lore = _"Text needed",
		-- TRANSLATORS: Lore author helptext for a building
		lore_author = _"Source needed",
		-- TRANSLATORS: Purpose helptext for a building
		purpose = _"Digs iron ore out of the ground in mountain terrain.",
		-- #TRANSLATORS: Note helptext for a building
		note = "",
		-- TRANSLATORS: Performance helptext for a building
		performance = _"Calculation needed"
   },

   animations = {
		idle = {
			pictures = path.list_directory(dirname, "idle_\\d+.png"),
			hotspot = { 50, 56 },
		},
		working = {
			pictures = path.list_directory(dirname, "idle_\\d+.png"), -- TODO(GunChleoc): No animation yet.
			hotspot = { 50, 56 },
		},
		empty = {
			pictures = path.list_directory(dirname, "empty_\\d+.png"),
			hotspot = { 50, 56 },
		},
	},

   aihints = {
		mines = "iron",
		prohibited_till = 1200
   },

	working_positions = {
		atlanteans_miner = 3
	},

   inputs = {
		bread_atlanteans = 10,
		smoked_fish = 10,
		smoked_meat = 6
	},
   outputs = {
		"iron_ore"
   },

	programs = {
		work = {
			-- TRANSLATORS: Completed/Skipped/Did not start mining iron because ...
			descname = _"mining iron",
			actions = {
				"sleep=45000",
				"return=skipped unless economy needs iron_ore",
				"consume=smoked_fish,smoked_meat:2 bread_atlanteans:2",
				"animate=working 20000",
				"mine=iron 4 100 5 2",
				"produce=iron_ore",
				"animate=working 20000",
				"mine=iron 4 100 5 2",
				"produce=iron_ore:2",
				"animate=working 20000",
				"mine=iron 4 100 5 2",
				"produce=iron_ore:2"
			}
		},
	},
	out_of_resource_notification = {
		title = _"Main Iron Vein Exhausted",
		message =
			_"This iron mine’s main vein is exhausted. Expect strongly diminished returns on investment." .. " " ..
			-- TRANSLATORS: "it" is a mine.
			_"You should consider dismantling or destroying it.",
		delay_attempts = 0
	},
}
