push_textdomain("world")

local dirname = path.dirname(__file__)

local terrain_affinity = include(dirname .. "../terrain_affinity.lua")

wl.Descriptions():new_immovable_type{
   name = "alder_summer_pole",
   descname = _("Alder (Pole)"),
   size = "small",
   animation_directory = dirname,
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:52s500ms",
         "remove=chance:7.42%",
         "grow=alder_summer_mature",
      },
   },
   spritesheets = {
      idle = {
         basename = "pole",
         fps = 8,
         frames = 4,
         rows = 2,
         columns = 2,
         hotspot = { 12, 28 }
      }
   },
}

pop_textdomain()
