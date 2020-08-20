dirname = path.dirname(__file__)

tribes:new_ship_type {
   msgctxt = "empire_ship",
   name = "empire_ship",
   -- TRANSLATORS: This is the Empire's ship's name used in lists of units
   descname = pgettext("empire_ship", "Ship"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   capacity = 30,
   vision_range = 4,

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 115, 100 },
         fps = 10
      },
      sail = {
         hotspot = { 115, 100 },
         fps = 10,
         directional = true
      }
   }
}
