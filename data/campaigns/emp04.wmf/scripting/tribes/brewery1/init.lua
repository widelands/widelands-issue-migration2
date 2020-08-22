dirname = "tribes/buildings/productionsites/empire/brewery/"

push_textdomain("scenario_emp04.wmf")

tribes:new_productionsite_type {
   msgctxt = "empire_building",
   name = "empire_brewery1",
   descname = pgettext("empire_building", "Brewery"),
   icon = dirname .. "menu.png",
   size = "medium",
   enhancement = "empire_brewery2",

   buildcost = {
      log = 1,
      planks = 2,
      granite = 2
   },
   return_on_dismantle = {
      planks = 1,
      granite = 1
   },


   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 39, 62 },
      },
      working = {
         pictures = path.list_files(dirname .. "idle_??.png"), -- TODO(GunChleoc): No animation yet.
         hotspot = { 39, 62 },
      },
   },

   aihints = {

   },

   working_positions = {
      empire_brewer = 1
   },

   inputs = {
      { name = "water", amount = 3 },
      { name = "wheat", amount = 7 }
   },

   programs = {
      main = {
         descname = "brewing beer",
         actions = {
            "sleep=30000",
            "return=skipped unless economy needs beer",
            "consume=water:3 wheat",
            "playsound=sound/empire/beerbubble 180",
            "animate=working duration:30s",
            "produce=beer"
         }
      },
   },
}
pop_textdomain()
