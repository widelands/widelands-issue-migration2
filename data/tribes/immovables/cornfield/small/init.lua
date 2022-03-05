push_textdomain("tribes")

dirname = path.dirname(__file__)

wl.Descriptions():new_immovable_type {
   name = "cornfield_small",
   -- TRANSLATORS: This is an immovable name used in lists of immovables
   descname = pgettext("immovable", "Cornfield (small)"),
   icon = dirname .. "menu.png",
   size = "small",
   programs = {
      main = {
         "animate=idle duration:45s",
         "transform=cornfield_medium",
      }
   },

   animation_directory = dirname,
   spritesheets = {
      idle = {
         frames = 5,
         rows = 3,
         columns = 2,
         hotspot = { 34, 24 }
      },
   }
}

pop_textdomain()
