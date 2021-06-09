push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_ware_type {
   name = "spear_stone_tipped",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Stone-tipped Spear"),
   icon = dirname .. "menu.png",

   animation_directory = dirname,
   animations = {
      idle = {
         hotspot = { 7, 10 },
      },
   }
}

pop_textdomain()
