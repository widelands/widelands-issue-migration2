dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "desert_rocks1",
   descname = _ "Rocks 1",
   animation_directory = dirname,
   editor_category = "rocks",
   size = "big",
   programs = {
      shrink = {
         "remove="
      }
   },
   animations = {
      idle = {
         basename = "rocks1"),
         hotspot = { 38, 82 }
      },
   }
}

world:new_immovable_type{
   name = "desert_rocks2",
   descname = _ "Rocks 2",
   animation_directory = dirname,
   editor_category = "rocks",
   size = "big",
   programs = {
      shrink = {
         "transform=desert_rocks1"
      }
   },
   animations = {
      idle = {
         basename = "rocks2"),
         hotspot = { 36, 86 }
      },
   }
}

world:new_immovable_type{
   name = "desert_rocks3",
   descname = _ "Rocks 3",
   animation_directory = dirname,
   editor_category = "rocks",
   size = "big",
   programs = {
      shrink = {
         "transform=desert_rocks2"
      }
   },
   animations = {
      idle = {
         basename = "rocks3"),
         hotspot = { 39, 90 }
      },
   }
}

world:new_immovable_type{
   name = "desert_rocks4",
   descname = _ "Rocks 4",
   animation_directory = dirname,
   editor_category = "rocks",
   size = "big",
   programs = {
      shrink = {
         "transform=desert_rocks3"
      }
   },
   animations = {
      idle = {
         basename = "rocks4"),
         hotspot = { 40, 84 }
      },
   }
}

world:new_immovable_type{
   name = "desert_rocks5",
   descname = _ "Rocks 5",
   animation_directory = dirname,
   editor_category = "rocks",
   size = "big",
   programs = {
      shrink = {
         "transform=desert_rocks4"
      }
   },
   animations = {
      idle = {
         basename = "rocks5"),
         hotspot = { 38, 89 }
      },
   }
}

world:new_immovable_type{
   name = "desert_rocks6",
   descname = _ "Rocks 6",
   animation_directory = dirname,
   editor_category = "rocks",
   size = "big",
   programs = {
      shrink = {
         "transform=desert_rocks5"
      }
   },
   animations = {
      idle = {
         basename = "rocks6"),
         hotspot = { 38, 91 }
      },
   }
}
