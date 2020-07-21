dirname = path.dirname(__file__)

terrain_affinity = {
   preferred_temperature = 50,
   preferred_humidity = 800,
   preferred_fertility = 450,
   pickiness = 80,
}

world:new_immovable_type{
   name = "larch_summer_sapling",
   descname = _ "Larch (Sapling)",
   editor_category = "trees_coniferous",
   size = "small",
   attributes = { "tree_sapling" },
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 60000",
         "transform=larch_summer_pole remove:44",
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
         hotspot = { 5, 13 }
      }
   },
}

world:new_immovable_type{
   name = "larch_summer_pole",
   descname = _ "Larch (Pole)",
   editor_category = "trees_coniferous",
   size = "small",
   attributes = {},
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 60000",
         "transform=larch_summer_mature remove:34",
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
         hotspot = { 10, 29 }
      }
   },
}

world:new_immovable_type{
   name = "larch_summer_mature",
   descname = _ "Larch (Mature)",
   editor_category = "trees_coniferous",
   size = "small",
   attributes = {},
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 55000",
         "transform=larch_summer_old remove:24",
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
         hotspot = { 13, 49 }
      }
   },
}

world:new_immovable_type{
   name = "larch_summer_old",
   descname = _ "Larch (Old)",
   species = _ "Larch",
   icon = dirname .. "menu.png",
   editor_category = "trees_coniferous",
   size = "small",
   attributes = { "tree" },
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 1455000",
         "transform=deadtree3 success:23",
         "seed=larch_summer_sapling 30",
      },
      fall = {
         "transform=",
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
         hotspot = { 16, 60 },
         sound_effect = {
            path = "sound/animals/bird6",
         },
      }
   },
}
