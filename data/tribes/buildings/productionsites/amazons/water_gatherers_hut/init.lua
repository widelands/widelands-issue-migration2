push_textdomain("tribes")

local dirname = path.dirname (__file__)

wl.Descriptions():new_productionsite_type {
   name = "amazons_water_gatherers_hut",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext ("amazons_building", "Water Gatherer’s Hut"),
   icon = dirname .. "menu.png",
   size = "small",

   buildcost = {
      log = 3,
      granite = 1,
      rubber = 1
   },
   return_on_dismantle = {
      log = 2,
   },

   animation_directory = dirname,
   animations = {
      idle = {hotspot = {39, 46}},
      unoccupied = {hotspot = {39, 46}}
   },

   aihints = {
      needs_water = true,
   },

   working_positions = {
      amazons_carrier = 1
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _("working"),
         actions = {
            -- time of worker: 4.6 sec
            -- radius: 8
            -- steps from building to tree: 2...11
            -- min. time total: 2 * 2 * 1.8 + 4.6 + 20 = 31.8 sec
            -- max. time total: 2 * 11 * 1.8 + 4.6 + 20 = 64.2 sec
            "sleep=duration:20s",
            "callworker=fetch_water",
            -- he carries 2 buckets so we need to create one now
            "produce=water",
         }
      },
   },

   out_of_resource_notification = {
      -- Translators: Short for "Out of ..." for a resource
      title = _("No Water"),
      heading = _("Out of Water"),
      message = pgettext ("amazons_building", "The carrier working at this water gatherer’s hut can’t find any water in its vicinity."),
      productivity_threshold = 33
   },
}

pop_textdomain()
