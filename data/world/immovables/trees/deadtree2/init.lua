dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "deadtree2",
   descname = _ "Dead Tree",
   editor_category = "trees_dead",
   size = "none",
   attributes = {},
   programs = {
      program = {
        "animate=idle 20000",
        "transform=remove:12"
      }
   },
   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 27, 56 },
      },
   }
}
