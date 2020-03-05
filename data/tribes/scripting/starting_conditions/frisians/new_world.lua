-- =======================================================================
--                 Start conditions for New World
-- =======================================================================

include "scripting/starting_conditions.lua"

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

   local fields = find_ocean_fields(7)
   -- items per expedition (incl. builder): 25
   local items = {
      {
         log = 1,
         brick = 2,
         granite = 2,
         reed = 3,
         frisians_claydigger = 1,
         frisians_soldier = 1,
         frisians_brickmaker = 1,
      },
      {
         brick = 1,
         iron = 6,
         frisians_blacksmith = 1,
         frisians_soldier = 1,
         frisians_geologist = 1,
         frisians_miner = 1,
         frisians_smelter = 1,
      },
      {
         log = 3,
         brick = 4,
         reed = 3,
         frisians_stonemason = 1,
         frisians_reed_farmer = 1,
      },
      {
         brick = 2,
         reed = 2,
         granite = 2,
         iron = 2,
         frisians_soldier = 1,
         frisians_geologist = 1,
         frisians_miner = 1,
         frisians_smelter = 1,
      },
      {
         brick = 2,
         reed = 2,
         granite = 2,
         frisians_woodcutter = 3,
         frisians_forester = 2,
      },
      {
         log = 5,
         brick = 5,
         frisians_stonemason = 1,
      },
      {
         log = 2,
         brick = 2,
         reed = 2,
         granite = 2,
         frisians_smoker = 1,
         frisians_fisher = 1,
         frisians_stonemason = 1,
      },
   }
   for i,f in pairs(fields) do
      local ship = player:place_ship(f)
      if i > 4 then ship.capacity = 41 else ship.capacity = 42 end
      ship:make_expedition(items[i])
   end
   sleep_then_goto(1000, fields[1])
end
}

return init
