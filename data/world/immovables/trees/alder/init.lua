dirname = path.dirname(__file__)

terrain_affinity = {
   -- Temperature is in arbitrary units.
   preferred_temperature = 125,

   -- Value between 0 and 1000 (1000 being very wet).
   preferred_humidity = 650,

   -- Values between 0 and 1000 (1000 being very fertile).
   preferred_fertility = 600,

   -- A value in [0, 100] that defines how well this can deal with non-ideal
   -- situations. Lower means it is less picky, i.e. it can deal better.
   pickiness = 60,
}

world:new_immovable_type{
   name = "alder_summer_sapling",
   descname = _ "Alder (Sapling)",
   editor_category = "trees_deciduous",
   size = "small",
   attributes = { "tree_sapling" },
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 57500",
         "remove=21",
         "transform=alder_summer_pole",
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
   name = "alder_summer_pole",
   descname = _ "Alder (Pole)",
   editor_category = "trees_deciduous",
   size = "small",
   attributes = {},
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 52500",
         "remove=19",
         "transform=alder_summer_mature",
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
   name = "alder_summer_mature",
   descname = _ "Alder (Mature)",
   editor_category = "trees_deciduous",
   size = "small",
   attributes = {},
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 50000",
         "remove=18",
         "transform=alder_summer_old",
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
   name = "alder_summer_old",
   descname = _ "Alder (Old)",
   species = _ "Alder",
   icon = dirname .. "menu.png",
   editor_category = "trees_deciduous",
   size = "small",
   attributes = { "tree" },
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 1550000",
         "transform=deadtree4 5",
         "seed=alder_summer_sapling 180",
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
         hotspot = { 24, 60 },
         sound_effect = {
            path = "sound/animals/bird4",
         }
      }
   },
}
