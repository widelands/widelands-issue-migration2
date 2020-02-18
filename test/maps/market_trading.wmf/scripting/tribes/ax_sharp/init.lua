dirname = "test/maps/market_trading.wmf/" .. path.dirname(__file__)

-- Test that replacing a ware works

tribes:new_ware_type {
   msgctxt = "ware",
   name = "ax_sharp",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Replaced Ware"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   default_target_quantity = {
      barbarians = 1
   },
   preciousness = {
      barbarians = 1
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 7, 7 },
      },
   }
}
