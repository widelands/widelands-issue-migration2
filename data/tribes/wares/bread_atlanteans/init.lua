push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_ware_type {
   name = "atlanteans_bread",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("atlanteans_ware", "Bread"),
   icon = dirname .. "menu.png",

   animation_directory = dirname,
   animations = {
      idle = {
         hotspot = { 9, 13 },
      },
   }
}

pop_textdomain()
