push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_productionsite_type {
   name = "europeans_ironmine_basic",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("europeans_building", "Basic Iron Mine"),
   icon = dirname .. "menu.png",
   size = "mine",

   buildcost = {
      log = 4,
      planks = 2,
      reed = 2
   },
   return_on_dismantle = {
      log = 4
   },

   --enhancement = {
      --name = "europeans_ironmine_level_1",
      --enhancement_cost = {
      --log = 2,
      --planks = 1,
      --reed = 1
      
      --},
      --enhancement_return_on_dismantle = {
         --log = 2
      --}
   --},

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 21, 36 },
      },
      build = {
         pictures = path.list_files(dirname .. "build_??.png"),
         hotspot = { 21, 36 },
      },
      working = {
         pictures = path.list_files(dirname .. "working_??.png"),
         hotspot = { 21, 36 },
      },
      empty = {
         pictures = path.list_files(dirname .. "empty_??.png"),
         hotspot = { 21, 36 },
      },
   },

   aihints = {
   },

   working_positions = {
      europeans_miner_basic = 1
   },

   inputs = {
      { name = "smoked_fish", amount = 4 },
      { name = "smoked_meat", amount = 4 },
      { name = "europeans_bread", amount = 8 }
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start mining iron because ...
         descname = _"mining iron",
         actions = {
            "return=skipped unless economy needs ore",
            "consume=smoked_fish,smoked_meat europeans_bread",
            "sleep=duration:45s",
            "animate=working duration:20s",
            "mine=resource_iron radius:2 yield:33.33% when_empty:5% experience_on_fail:17%",
            "produce=ore"
         }
      },
   },
   out_of_resource_notification = {
      -- Translators: Short for "Out of ..." for a resource
      title = _"No Iron",
      heading = _"Main Iron Vein Exhausted",
      message =
         pgettext("europeans_building", "This iron mine’s main vein is exhausted. Expect strongly diminished returns on investment. You should consider enhancing, dismantling or destroying it."),
   },
}

pop_textdomain()
