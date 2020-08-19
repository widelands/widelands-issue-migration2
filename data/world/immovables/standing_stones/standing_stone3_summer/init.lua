dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "standing_stone3_summer",
   descname = _ "Standing Stone",
   size = "big",
   programs = {},
   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 25, 28 },
      },
   }
}
