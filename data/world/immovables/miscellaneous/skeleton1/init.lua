push_textdomain("world")

dirname = path.dirname(__file__)

wl.Descriptions():new_immovable_type{
   name = "skeleton1",
   descname = _("Skeleton"),
   size = "none",
   programs = {},
   animation_directory = dirname,
   animations = {
      idle = {
         hotspot = { 29, 36 },
      },
   }
}

pop_textdomain()
