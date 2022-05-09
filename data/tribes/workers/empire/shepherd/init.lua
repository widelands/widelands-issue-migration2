push_textdomain("tribes")

dirname = path.dirname(__file__)

wl.Descriptions():new_worker_type {
   name = "empire_shepherd",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("empire_worker", "Shepherd"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      empire_carrier = 1
   },

   animations = {
      idle = {
         hotspot = { 16, 29 }
      },
   },

   spritesheets = {
      walk = {
         fps = 20,
         frames = 20,
         rows = 5,
         columns = 4,
         directional = true,
         hotspot = { 26, 30 }
      },
      walkload = {
         basename = "walk",
         fps = 20,
         frames = 20,
         rows = 5,
         columns = 4,
         directional = true,
         hotspot = { 26, 30 }
      },
   },
}

pop_textdomain()
