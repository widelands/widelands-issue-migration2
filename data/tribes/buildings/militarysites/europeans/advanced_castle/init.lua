push_textdomain("tribes")

dirname = path.dirname(__file__)

tribes:new_militarysite_type {
   msgctxt = "europeans_building",
   name = "europeans_advanced_castle",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("europeans_building", "Advanced Castle"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "big",

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 94, 106 }
      }
   },

   aihints = {
      expansion = true,
      fighting = true,
      mountain_conqueror = true,
      prohibited_till = 9600
   },

   max_soldiers = 24,
   heal_per_second = 300,
   conquers = 24,
   prefer_heroes = true,

   messages = {
      -- TRANSLATORS: Message sent by an Europeans military site
      occupied = pgettext("europeans_building", "Your soldiers have occupied your castle."),
      -- TRANSLATORS: Message sent by an Europeans military site
      aggressor = pgettext("europeans_building", "Your castle discovered an aggressor."),
      -- TRANSLATORS: Message sent by an Europeans military site
      attack = pgettext("europeans_building", "Your castle is under attack."),
      -- TRANSLATORS: Message sent by an Europeans military site
      defeated_enemy = pgettext("europeans_building", "The enemy defeated your soldiers at the castle."),
      -- TRANSLATORS: Message sent by an Europeans military site
      defeated_you = pgettext("europeans_building", "Your soldiers defeated the enemy at the castle.")
   },
}

pop_textdomain()
