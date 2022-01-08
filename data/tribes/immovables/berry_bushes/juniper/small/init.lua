push_textdomain("tribes")

dirname = path.dirname (__file__)

include(dirname .. "../terrain_affinity.lua")

descriptions:new_immovable_type {
   name = "berry_bush_juniper_small",
   descname = _ "Juniper (small)",
   size = "small",
   icon = dirname .. "menu.png",
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:10s",
         "grow=berry_bush_juniper_medium",
      },
   },
   animation_directory = dirname,
   spritesheets = {
      idle = {
         hotspot = {6, 30},
         frames = 4,
         columns = 2,
         rows = 2,
      }
   }
}

pop_textdomain()
