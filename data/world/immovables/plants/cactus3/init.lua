dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "cactus3",
   descname = _ "Cactus",
   size = "none",
   programs = {},
   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 10, 71 },
      },
   }
}
