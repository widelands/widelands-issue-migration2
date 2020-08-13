dirname = path.dirname(__file__)

tribes:new_worker_type {
   msgctxt = "barbarians_worker",
   name = "barbarians_fisher",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("barbarians_worker", "Fisher"),
   helptext_script = dirname .. "helptexts.lua",
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      barbarians_carrier = 1,
      fishing_rod = 1
   },

   programs = {
      fish = {
         "findspace=size:any radius:7 resource:fish",
         "walk=coords",
         "playsound=sound/fisher/fisher_throw_net 192",
         "mine=fish radius:1",
         "animate=fish duration:10s500ms",
         "playsound=sound/fisher/fisher_pull_net 192",
         "createware=fish",
         "return"
      }
   },

   animations = {
      idle = {
         hotspot = { 7, 33 },
      },
   },
   spritesheets = {
      walk = {
         fps = 10,
         frames = 10,
         rows = 4,
         columns = 3,
         directional = true,
         hotspot = { 8, 32 }
      },
      -- TODO(GunChleoc): Needs walkload animation, or some ware hotspot/sizing magic.
      fish = {
         fps = 10,
         frames = 30,
         rows = 6,
         columns = 5,
         hotspot = { 7, 33 }
      }
   }
}
