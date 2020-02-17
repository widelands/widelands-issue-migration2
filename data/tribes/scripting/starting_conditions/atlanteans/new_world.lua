-- =======================================================================
--                 Start conditions for New World
-- =======================================================================

include "scripting/ui.lua"

set_textdomain("tribes")

init = {
   -- TRANSLATORS: This is the name of a starting condition
   descname = _ "New World",
   -- TRANSLATORS: This is the tooltip for the "New World" starting condition
   tooltip = _"Start the game with seven ships full of wares on the ocean",
   map_tags = {"seafaring"},

   func = function(player, shared_in_start)

   local map = wl.Game().map
   local sf = map.player_slots[player.number].starting_field
   if shared_in_start then
      sf = shared_in_start
   else
      player:allow_workers("all")
   end

   local fields = {}
   repeat
      local f = map:get_field(math.random(map.width), math.random(map.height))
      if not f:has_caps("swimmable") then
         f = nil
      else
         local route_found = false
         for i,port in pairs(map.port_spaces) do
            if map:sea_route_exists(f, map:get_field(port.x, port.y)) then
               route_found = true
               break
            end
         end
         if not route_found then f = nil end
      end
      if f then table.insert(fields, f) end
   until #fields == 7

   -- items per expedition (incl. builder): 18
   local items = {
      {
         log = 4,
         granite = 2,
         planks = 2,
         spidercloth = 1,
         atlanteans_soldier = 1,
      },
      {
         granite = 2,
         planks = 2,
         spidercloth = 1,
         atlanteans_soldier = 1,
         atlanteans_geologist = 1,
         atlanteans_miner = 2,
      },
      {
         spidercloth = 1,
         atlanteans_soldier = 1,
         atlanteans_stonecutter = 1,
         iron = 4,
         atlanteans_smelter = 2,
         atlanteans_toolsmith = 1,
      },
      {
         log = 6,
         granite = 2,
         atlanteans_stonecutter = 1,
      },
      {
         log = 4,
         spidercloth = 1,
         atlanteans_geologist = 1,
         atlanteans_miner = 3,
      },
      {
         log = 1,
         granite = 2,
         planks = 2,
         atlanteans_woodcutter = 3,
         atlanteans_forester = 2,
      },
      {
         granite = 2,
         planks = 2,
         atlanteans_stonecutter = 1,
         atlanteans_baker = 1,
         atlanteans_smoker = 2,
         atlanteans_fisher = 1,
      },
   }
   for i,f in pairs(fields) do
      local ship = player:place_ship(f)
      ship:make_expedition(items[i])
   end
   scroll_to_field(fields[1])
end
}

return init
