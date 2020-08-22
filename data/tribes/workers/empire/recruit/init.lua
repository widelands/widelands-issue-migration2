dirname = path.dirname(__file__)

tribes:new_worker_type {
   msgctxt = "empire_worker",
   name = "empire_recruit",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("empire_worker", "Recruit"),
   helptext_script = dirname .. "helptexts.lua",
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {}, -- This will give the worker the property "buildable"

   animations = {
      idle = {
         hotspot = { 10, 30 },
         fps = 5
      },
      walk = {
         hotspot = { 10, 30 },
         fps = 10,
         directional = true
      }
   }
}
