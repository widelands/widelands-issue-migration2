push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_worker_type {
   name = "europeans_charcoal_burner_advanced",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("europeans_worker", "Advanced Charcoal Burner"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,
   
   buildcost = {
      europeans_carrier = 1,
      shovel = 2,
      buckets = 2,
      hook_pole = 1
   },

   animations = {
      idle = {
         hotspot = { 11, 23 }
      },
      walk = {
         hotspot = { 21, 28 },
         fps = 10,
         directional = true
      },
      walkload = {
         hotspot = { 17, 27 },
         fps = 10,
         directional = true
      }
   }
}

pop_textdomain()
