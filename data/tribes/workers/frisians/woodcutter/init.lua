dirname = path.dirname (__file__)

tribes:new_worker_type {
   msgctxt = "frisians_worker",
   name = "frisians_woodcutter",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext ("frisians_worker", "Woodcutter"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      frisians_carrier = 1,
      felling_ax = 1
   },

   programs = {
      harvest = {
         "findobject=attrib:tree radius:10",
         "walk=object",
         "playsound=sound/woodcutting/woodcutting 255",
         "animate=hacking 20000",
         "playsound=sound/woodcutting/tree_falling 130",
         "callobject=fall",
         "animate=idle 2000",
         "createware=log",
         "return"
      }
   },

   ware_hotspot = {0, 20},

   spritesheets = {
      walk = {
         directory = dirname,
         basename = "walk",
         fps = 15,
         frames = 10,
         columns = 5,
         rows = 2,
         directional = true,
         hotspot = {16, 23}
      },
      walkload = {
         directory = dirname,
         basename = "walkload",
         fps = 15,
         frames = 10,
         columns = 5,
         rows = 2,
         directional = true,
         hotspot = {12, 26}
      },
      idle = {
         directory = dirname,
         basename = "idle",
         fps = 10,
         frames = 20,
         columns = 5,
         rows = 4,
         hotspot = {9, 23}
      },
      hacking = {
         directory = dirname,
         basename = "hacking",
         fps = 10,
         frames = 10,
         columns = 5,
         rows = 2,
         hotspot = {14, 23}
      },
   },
}
