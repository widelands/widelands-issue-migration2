push_textdomain("world")

local dirname = path.dirname(__file__)

wl.Descriptions():new_immovable_type{
   name = "standing_stone5_desert",
   descname = _("Standing Stone"),
   size = "big",
   programs = {},
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   animations = {
      idle = {
         hotspot = { 37, 27 },
      },
   }
}

pop_textdomain()
