dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "standing_stone7",
   descname = _ "Standing Stone",
   size = "none",
   programs = {},
   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 7, 63 },
      },
   }
}
