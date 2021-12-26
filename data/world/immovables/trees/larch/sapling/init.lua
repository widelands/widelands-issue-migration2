push_textdomain("world")

dirname = path.dirname(__file__)

terrain_affinity = {
   preferred_temperature = 50,
   preferred_humidity = 800,
   preferred_fertility = 450,
   pickiness = 80,
}

wl.Descriptions():new_immovable_type{
   name = "larch_summer_sapling",
   descname = _ "Larch (Sapling)",
   size = "small",
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:1m",
         "remove=chance:17.19%",
         "grow=larch_summer_pole",
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
         hotspot = { 4, 12 }
      }
   },
}

pop_textdomain()
