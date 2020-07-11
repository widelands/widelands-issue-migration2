dirname = path.dirname(__file__)

tribes:new_ware_type {
   msgctxt = "ware",
   name = "sword_short",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Short Sword"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",

   animations = {
      idle = {
         hotspot = { 12, 9 },
      }
   },
}
