dirname = path.dirname(__file__)

tribes:new_warehouse_type {
   msgctxt = "barbarians_building",
   name = "barbarians_warehouse",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("barbarians_building", "Warehouse"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "medium",

   buildcost = {
      log = 3,
      blackwood = 2,
      granite = 2,
      grout = 3,
      thatch_reed = 1
   },
   return_on_dismantle = {
      log = 1,
      blackwood = 1,
      granite = 1,
      grout = 1
   },

   animations = {
      idle = {
         mipmap = {
            {
               scale = 0.5,
               pictures = path.list_files(dirname .. "idle_0.5_??.png"),
               hotspot = { 30, 39 },
            },
            {
               scale = 1,
               pictures = path.list_files(dirname .. "idle_1_??.png"),
               hotspot = { 60, 78 },
            },
            {
               scale = 2,
               pictures = path.list_files(dirname .. "idle_2_??.png"),
               hotspot = { 120, 156 },
            }
         }
      },
      build = {
         pictures = path.list_files(dirname .. "build_??.png"),
         hotspot = { 60, 78 },
      }
   },

   aihints = {},

   heal_per_second = 170,
}
