dirname = path.dirname (__file__)

tribes:new_worker_type {
   msgctxt = "amazons_worker",
   name = "amazons_scout",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext ("amazons_worker", "Scout"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      amazons_carrier = 1
   },

   programs = {
      scout = {
         "scout=15 75000", -- radius 15, 75 seconds until return
         "return"
      }
   },

   ware_hotspot = {0, 29},
   spritesheets = {
      walk = {
         directory = dirname,
         basename = "walk",
         directional = true,
         hotspot = {17, 28},
         fps = 15,
         frames = 30,
         columns = 6,
         rows = 5
      },
      walkload = {
         directory = dirname,
         basename = "walkload",
         directional = true,
         hotspot = {17, 31},
         fps = 15,
         frames = 30,
         columns = 6,
         rows = 5
      },
      idle = {
         directory = dirname,
         basename = "walk_se",
         hotspot = {17, 28},
         fps = 15,
         frames = 30,
         columns = 6,
         rows = 5
      },
   },
}
