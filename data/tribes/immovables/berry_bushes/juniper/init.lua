dirname = path.dirname (__file__)

terrain_affinity = {
   preferred_temperature = 115,
   preferred_humidity = 600,
   preferred_fertility = 200,
   pickiness = 15,
}

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "berry_bush_juniper_tiny",
   descname = _ "Juniper (tiny)",
   size = "small",
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "tiny/menu.png",
   attributes = { "seed_berrybush" },
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 10000",
         "remove=60",
         "grow=berry_bush_juniper_small",
      },
   },
   animations = {
      idle = {
         directory = dirname .. "tiny",
         basename = "idle",
         hotspot = {3, 15}
      }
   }
}

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "berry_bush_juniper_small",
   descname = _ "Juniper (small)",
   size = "small",
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "small/menu.png",
   attributes = {},
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 10000",
         "remove=20",
         "grow=berry_bush_juniper_medium",
      },
   },
   animations = {
      idle = {
         directory = dirname .. "small",
         basename = "idle",
         hotspot = {6, 30}
      }
   }
}

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "berry_bush_juniper_medium",
   descname = _ "Juniper (medium)",
   size = "small",
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "medium/menu.png",
   attributes = { "flowering" },
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 70000",
         "remove=10",
         "grow=berry_bush_juniper_ripe",
      },
   },
   animations = {
      idle = {
         directory = dirname .. "medium",
         basename = "idle",
         hotspot = {10, 47}
      }
   }
}

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "berry_bush_juniper_ripe",
   descname = _ "Juniper (ripe)",
   size = "small",
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "ripe/menu.png",
   attributes = { "ripe_bush" },
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 800000",
         "remove=",
      },
      harvest = {
         "remove=",
      }
   },
   animations = {
      idle = {
         directory = dirname .. "ripe",
         basename = "idle",
         hotspot = {11, 49}
      }
   }
}
