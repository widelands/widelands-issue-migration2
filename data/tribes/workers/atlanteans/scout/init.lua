dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_files(dirname .. "idle_??.png"),
      hotspot = { 4, 22 },
   }
}
add_directional_animation(animations, "walk", dirname, "walk", {9, 25}, 10)

tribes:new_worker_type {
   msgctxt = "atlanteans_worker",
   name = "atlanteans_scout",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("atlanteans_worker", "Scout"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 3,

   buildcost = {
      atlanteans_carrier = 1
   },

   programs = {
      scout = {
         "scout=radius:15 duration:1m15s",
         "return"
      }
   },

   animations = animations,
}
