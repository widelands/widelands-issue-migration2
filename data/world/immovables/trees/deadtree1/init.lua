dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "deadtree1",
   descname = _ "Dead Tree",
   editor_category = "trees_dead",
   size = "none",
   attributes = {},
   programs = {
      program = {
         "animate=idle duration:20s",
         "remove=success:16"
      }
   },
   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 25, 56 },
      },
   }
}
