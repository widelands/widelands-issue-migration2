push_textdomain("world")

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

wl.Descriptions():new_immovable_type{
   name = "alder_summer_sapling",
   descname = _ "Alder (Sapling)",
   size = "small",
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:57s500ms",
         "remove=chance:8.2%",
         "grow=alder_summer_pole",
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
         hotspot = { 5, 12 }
      }
   },
}

pop_textdomain()
