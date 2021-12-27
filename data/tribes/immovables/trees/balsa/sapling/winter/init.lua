push_textdomain("tribes")

dirname = path.dirname(__file__) .. "../"

include(dirname .. "../terrain_affinity.lua")

descriptions:new_immovable_type{
   name = "balsa_winter_amazons_sapling",
   -- TRANSLATORS:  This is an amazon rare tree.
   descname = _ "Balsa Tree (Sapling)",
   size = "small",
   terrain_affinity = terrain_affinity_winter,
   programs = {
      main = {
         "animate=idle duration:55s",
         "remove=chance:13.67%",
         "grow=balsa_winter_amazons_pole",
      },
   },
   icon = dirname .. "menu_sapling.png",
   animation_directory = dirname,
   spritesheets = spritesheet_sapling
}

pop_textdomain()
