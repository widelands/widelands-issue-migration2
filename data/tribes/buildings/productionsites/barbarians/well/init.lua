dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "barbarians_building",
   name = "barbarians_well",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("barbarians_building", "Well"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "small",

   buildcost = {
      log = 4
   },
   return_on_dismantle = {
      log = 2
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 19, 33 },
      },
      build = {
         pictures = path.list_files(dirname .. "build_??.png"),
         hotspot = { 19, 33 },
      },
      working = {
         pictures = path.list_files(dirname .. "working_??.png"),
         hotspot = { 19, 33 },
      },
   },

   aihints = {
      collects_ware_from_map = "water",
      basic_amount = 1
   },

   working_positions = {
      barbarians_carrier = 1
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _"working",
         actions = {
            "sleep=duration:20s",
            "animate=working duration:20s",
            "mine=water workarea:1 resources:100% depleted:65% experience:2%",
            "produce=water",
         }
      },
   },

   out_of_resource_notification = {
      -- Translators: Short for "Out of ..." for a resource
      title = _"No Water",
      heading = _"Out of Water",
      message = pgettext("barbarians_building", "The carrier working at this well can’t find any water in his well."),
      productivity_threshold = 33
   },
}
