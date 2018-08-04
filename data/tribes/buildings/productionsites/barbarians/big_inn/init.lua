dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "barbarians_building",
   name = "barbarians_big_inn",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("barbarians_building", "Big Inn"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "medium",

   enhancement_cost = {
      log = 1,
      grout = 3,
      thatch_reed = 2
   },
   return_on_dismantle_on_enhanced = {
      grout = 2
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 57, 88 },
      },
      build = {
         pictures = path.list_files(dirname .. "build_??.png"),
         hotspot = { 57, 88 },
      },
      working = {
         pictures = path.list_files(dirname .. "working_??.png"),
         hotspot = { 57, 88 },
      },
   },

   aihints = {
      prohibited_till = 930,
   },

   working_positions = {
      barbarians_innkeeper = 2
   },

   inputs = {
      { name = "fish", amount = 4 },
      { name = "meat", amount = 4 },
      { name = "barbarians_bread", amount = 4 },
      { name = "beer", amount = 4 },
      { name = "beer_strong", amount = 4 }
   },
   outputs = {
      "ration",
      "snack",
      "meal"
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _"working",
         actions = {
            -- ration, snack and meal needed
            -- time total: 120.8
            -- average ration: 120.8
            -- average snack: 120.8
            -- average meal: 120.8
            
            -- only ration and snack needed
            -- time total: 77.2
            -- average ration: 77.2
            -- average snack: 77.2
            
            -- only ration and meal needed
            -- time total: 80.2
            -- average ration: 80.2
            -- average meal: 80.2
            
            -- only snack and meal needed
            -- time total: 84.2
            -- average snack: 84.2
            -- average meal: 84.2
            
            -- only ration needed
            -- time total: 36.6
            -- average: 36.6
            
            -- only snack needed
            -- time total: 40.6
            -- average: 40.6
            
            -- only meal needed
            -- time total: 43.6
            -- average: 43.6
            "call=produce_ration",
            "call=produce_snack",
            "call=produce_meal",
            "return=skipped"
         }
      },
      produce_ration = {
         -- TRANSLATORS: Completed/Skipped/Did not start preparing a ration because ...
         descname = _"preparing a ration",
         actions = {
            -- time total: 36.6
            -- average: 36.6
            "return=skipped unless economy needs ration",
            "sleep=23000",
            "consume=barbarians_bread,fish,meat",
            "playsound=sound/barbarians/taverns tavern 100",
            "sleep=10000",
            "produce=ration"
         }
      },
      produce_snack = {
         -- TRANSLATORS: Completed/Skipped/Did not start preparing a snack because ...
         descname = _"preparing a snack",
         actions = {
            -- time total: 40.6
            -- average: 40.6
            "return=skipped unless economy needs snack",
            "sleep=5000",
            "consume=barbarians_bread fish,meat beer",
            "playsound=sound/barbarians/taverns biginn 100",
            "sleep=32000",
            "produce=snack"
         }
      },
      produce_meal = {
         -- TRANSLATORS: Completed/Skipped/Did not start preparing a meal because ...
         descname = _"preparing a meal",
         actions = {
            -- time total: 43.6
            -- average: 43.6
            "return=skipped unless economy needs meal",
            "sleep=5000",
            "consume=barbarians_bread fish,meat beer_strong",
            "playsound=sound/barbarians/taverns biginn 100",
            "sleep=35000",
            "produce=meal"
         }
      },
   },
}
