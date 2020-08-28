dirname = path.dirname(__file__)

tribes:new_ware_type {
   msgctxt = "ware",
   name = "marble_column",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Marble Column"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 3, 9 },
      },
   }
}
