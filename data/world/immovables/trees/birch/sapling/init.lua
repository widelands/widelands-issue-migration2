push_textdomain("world")

local dirname = path.dirname(__file__)

local terrain_affinity = include(dirname .. "../terrain_affinity.lua")

wl.Descriptions():new_immovable_type{
   name = "birch_summer_sapling",
   descname = _("Birch (Sapling)"),
   size = "small",
   animation_directory = dirname,
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:42s",
         "remove=chance:12.5%",
         "grow=birch_summer_pole",
      },
   },
   spritesheets = {
      idle = {
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
