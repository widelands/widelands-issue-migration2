dirname = path.dirname(__file__)

tribes:new_worker_type {
   msgctxt = "barbarians_worker",
   name = "barbarians_gardener",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("barbarians_worker", "Gardener"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      barbarians_carrier = 1,
      shovel = 1
   },

   programs = {
      plant = {
         "findspace=size:any radius:1",
         "walk=coords",
         "animate=plant 6500",
         "plant=attrib:seed_reed",
         "animate=plant 6500",
         "return"
      },
      harvest = {
         "findobject=attrib:ripe_reed radius:1",
         "walk=object",
         "animate=harvest 14000",
         "callobject=harvest",
         "createware=reed",
         "return"
      },
   },

   animations = {
      idle = {
         directory = dirname,
         hotspot = { -4, 11 }
      }
   },
   spritesheets = {
      walk = {
         directory = dirname,
         fps = 10,
         frames = 10,
         rows = 4,
         columns = 3,
         directional = true,
         hotspot = { 8, 23 }
      },
      -- TODO(GunChleoc): Walkload needs mipmap
      walkload = {
         directory = dirname,
         fps = 10,
         frames = 10,
         rows = 4,
         columns = 3,
         directional = true,
         hotspot = { 7, 23 }
      },
      plant = {
         directory = dirname,
         fps = 10,
         frames = 15,
         rows = 5,
         columns = 3,
         hotspot = { 10, 21 }
      },
      harvest = {
         directory = dirname,
         fps = 5,
         frames = 20,
         rows = 5,
         columns = 4,
         hotspot = { 10, 22 }
      }
   }
}
