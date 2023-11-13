push_textdomain("tribes")

local dirname = path.dirname(__file__)

wl.Descriptions():new_productionsite_type {
   name = "empire_well",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("empire_building", "Well"),
   icon = dirname .. "menu.png",
   size = "small",

   buildcost = {
      log = 2,
      granite = 1,
      marble = 1
   },
   return_on_dismantle = {
      log = 1,
      marble = 1
   },

   animation_directory = dirname,
   spritesheets = {
      idle = {
         frames = 1,
         columns = 1,
         rows = 1,
         hotspot = { 39, 57 },
      },
      working = {
         basename = "idle", -- TODO(GunChleoc): No animation yet.
         frames = 1,
         columns = 1,
         rows = 1,
         hotspot = { 39, 57 },
      },
   },

   aihints = {},

   working_positions = {
      empire_carrier = 1
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _("working"),
         actions = {
            -- time total: 20 + 20 + 3.6 = 43.6 sec
            -- average time if water resource is depleted:
            --   (65 * (20 + 20 + 3.6) + 35 * (20 + 20 + 10)) / 65 = 70.523 sec
            -- number 10 is measured experimentally
            "sleep=duration:20s",
            "animate=working duration:20s",
            "mine=resource_water radius:1 yield:100% when_empty:65%",
            "produce=water"
         }
      },
   },
   out_of_resource_notification = {
      -- Translators: Short for "Out of ..." for a resource
      title = _("No Water"),
      heading = _("Out of Water"),
      message = pgettext("empire_building", "The carrier working at this well can’t find any water in his well."),
      productivity_threshold = 33
   },
}

pop_textdomain()
