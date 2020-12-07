push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_productionsite_type {
   name = "europeans_bakery_basic",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("europeans_building", "Basic Bakery"),
   icon = dirname .. "menu.png",
   size = "medium",

   enhancement = {
        name = "europeans_bakery_normal",
        enhancement_cost = {
          planks = 2,
          brick = 2,
          grout = 1,
          spidercloth = 1,
          quartz = 1
        },
        enhancement_return_on_dismantle = {
          granite = 1,
          quartz = 1,
        },
   },

   buildcost = {
      reed = 2,
      planks = 2,
      brick = 3,
      grout = 3
   },
   return_on_dismantle = {
      log = 4,
      granite = 3
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 41, 58 },
      },
      unoccupied = {
         pictures = path.list_files(dirname .. "unoccupied_??.png"),
         hotspot = { 41, 58 },
      },
      working = {
         pictures = path.list_files(dirname .. "idle_??.png"), -- TODO(GunChleoc): No animation yet.
         hotspot = { 41, 58 },
      },
   },

   aihints = {
      prohibited_till = 1800
   },

   working_positions = {
      europeans_baker_basic = 1
   },

   inputs = {
      { name = "water", amount = 6 },
      { name = "cornmeal", amount = 6 }
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _"working",
         actions = {
            "call=baking_bread_basic",
            "call=baking_bread",
         }
      },
      baking_bread_basic = {
         -- TRANSLATORS: Completed/Skipped/Did not start baking bread because ...
         descname = pgettext("europeans_building", "baking pitta bread"),
         actions = {
            "consume=water cornmeal",
            "sleep=duration:60s",
            "animate=working duration:60s",
            "produce=europeans_bread"
         }
      },
      baking_bread = {
         -- TRANSLATORS: Completed/Skipped/Did not start baking bread because ...
         descname = pgettext("europeans_building", "baking pitta bread"),
         actions = {
            "return=skipped unless economy needs europeans_bread",
            "return=skipped when economy needs water",
            "consume=water:4 cornmeal:4",
            "sleep=duration:40s",
            "animate=working duration:40s",
            "produce=europeans_bread:2",
            "animate=working duration:40s",
            "produce=europeans_bread:2"
         }
      },
   },
}

pop_textdomain()
