dirname = path.dirname(__file__)

animations = {}
add_animation(animations, "idle", dirname, "idle", {12, 9})

tribes:new_ware_type {
   msgctxt = "ware",
   name = "sword_long",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Long Sword"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",

   animations = animations,
}
