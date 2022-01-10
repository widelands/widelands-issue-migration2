push_textdomain("tribes")

dirname = path.dirname(__file__) .. "../"

descriptions:new_immovable_type {
   name = "atlanteans_resi_gold_2",
   -- TRANSLATORS: This is a resource name used in lists of resources
   descname = pgettext("resource_indicator", "A Lot of Gold"),
   icon = dirname .. "pics/gold_much.png",
   programs = {
      main = {
         "animate=idle duration:10m",
         "remove="
      }
   },
   spritesheets = {
      idle = {
         directory = dirname .. "pics",
         basename = "gold_much",
         hotspot = {10, 36},
         frames = 4,
         columns = 2,
         rows = 2
      }
   }
}

pop_textdomain()
