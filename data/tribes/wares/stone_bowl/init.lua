push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_ware_type {
   name = "stone_bowl",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Stone Bowl"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",

   animations = {
      idle = {
         hotspot = { 6, 6 },
      }
   },
}

pop_textdomain()
