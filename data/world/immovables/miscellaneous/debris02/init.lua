dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "debris02",
   descname = _ "Debris",
   size = "small",
   programs = {},
   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 35, 35 },
      },
   }
}
