dirname = path.dirname(__file__)

terrain_affinity = {
   preferred_temperature = 150,
   preferred_humidity = 550,
   preferred_fertility = 500,
   pickiness = 80,
}

world:new_immovable_type{
   name = "palm_oil_desert_sapling",
   descname = _ "Oil Palm (Sapling)",
   editor_category = "trees_palm",
   size = "small",
   attributes = { "tree_sapling" },
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle duration:42s",
         "remove=32",
         "grow=palm_oil_desert_pole",
      },
   },
   spritesheets = {
      idle = {
         directory = dirname,
         basename = "sapling",
         fps = 8,
         frames = 4,
         rows = 2,
         columns = 2,
         hotspot = { 6, 13 }
      }
   },
}

world:new_immovable_type{
   name = "palm_oil_desert_pole",
   descname = _ "Oil Palm (Pole)",
   editor_category = "trees_palm",
   size = "small",
   attributes = {},
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle duration:40s",
         "remove=25",
         "grow=palm_oil_desert_mature",
      },
   },
   spritesheets = {
      idle = {
         directory = dirname,
         basename = "pole",
         fps = 8,
         frames = 4,
         rows = 2,
         columns = 2,
         hotspot = { 13, 29 }
      }
   },
}

world:new_immovable_type{
   name = "palm_oil_desert_mature",
   descname = _ "Oil Palm (Mature)",
   editor_category = "trees_palm",
   size = "small",
   attributes = {},
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle duration:25s",
         "remove=10",
         "seed=palm_oil_desert_sapling 80",
         "animate=idle duration:30s",
         "remove=10",
         "grow=palm_oil_desert_old",

      },
   },
   spritesheets = {
      idle = {
         directory = dirname,
         basename = "mature",
         fps = 8,
         frames = 4,
         rows = 2,
         columns = 2,
         hotspot = { 19, 49 }
      }
   },
}

world:new_immovable_type{
   name = "palm_oil_desert_old",
   descname = _ "Oil Palm (Old)",
   species = _ "Oil Palm",
   icon = dirname .. "menu.png",
   editor_category = "trees_palm",
   size = "small",
   attributes = { "tree" },
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle duration:800s",
         "transform=deadtree5 50",
         "seed=palm_oil_desert_sapling 80",
      },
      fall = {
         "remove=",
      },
   },
   spritesheets = {
      idle = {
         directory = dirname,
         basename = "old",
         fps = 10,
         frames = 4,
         rows = 2,
         columns = 2,
         hotspot = { 25, 61 },
         sound_effect = {
            path = "sound/animals/crickets2",
         },
      }
   },
}
