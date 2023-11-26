push_textdomain("tribes")

local dirname = path.dirname(__file__)

wl.Descriptions():new_productionsite_type {
   name = "empire_foresters_house",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("empire_building", "Forester’s House"),
   icon = dirname .. "menu.png",
   size = "small",

   buildcost = {
      log = 1,
      planks = 1,
      granite = 1
   },
   return_on_dismantle = {
      planks = 1,
      granite = 1
   },

   animation_directory = dirname,
   spritesheets = {
      idle = {
         frames = 1,
         columns = 1,
         rows = 1,
         hotspot = { 54, 56 },
      },
   },

   aihints = {
      space_consumer = true,
      very_weak_ai_limit = 2,
      weak_ai_limit = 4
   },

   working_positions = {
      empire_forester = 1
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start planting trees because ...
         descname = _("planting trees"),
         actions = {
            -- time of worker: 13.2-34.8 sec, mean 23.784 sec
            -- min. time total: 13.2 + 10 = 23.2 sec
            -- max. time total: 34.8 + 10 = 44.8 sec
            -- mean time total: 23.784 + 10 = 33.784 sec
            "callworker=plant",
            "sleep=duration:10s"
         }
      },
   },
}

pop_textdomain()
