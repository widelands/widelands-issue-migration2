dirname = path.dirname (__file__)

tribes:new_worker_type {
   msgctxt = "frisians_worker",
   name = "frisians_geologist",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext ("frisians_worker", "Geologist"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      frisians_carrier = 1,
      hammer = 1
   },

   programs = {
      expedition = {
         "repeatsearch=15 5 search"
      },
      search = {
         "animate=hacking duration:3s",
         "animate=idle duration:1s",
         "animate=hacking duration:2s",
         "animate=idle duration:1s",
         "animate=hacking duration:3s",
         "findresources"
      }
   },


   spritesheets = {
      walk = {
         directory = dirname,
         basename = "walk",
         fps = 15,
         frames = 10,
         columns = 5,
         rows = 2,
         directional = true,
         hotspot = {11, 23}
      },
      idle = {
         directory = dirname,
         basename = "idle",
         fps = 10,
         frames = 10,
         columns = 5,
         rows = 2,
         hotspot = {8, 23}
      },
      hacking = {
         directory = dirname,
         basename = "hacking",
         fps = 10,
         frames = 10,
         columns = 5,
         rows = 2,
         hotspot = {9, 17}
      },
   },
}
