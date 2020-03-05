-- =======================================================================
--                 Start conditions for Discovery
-- =======================================================================

include "scripting/starting_conditions.lua"
include "scripting/ui.lua"

set_textdomain("tribes")

init = {
   -- TRANSLATORS: This is the name of a starting condition
   descname = _ "Discovery",
   -- TRANSLATORS: This is the tooltip for the "Discovery" starting condition
   tooltip = _"Start the game with three ships on the ocean and only a handful of supplies",
   map_tags = {"seafaring"},

   func = function(player, shared_in_start)

   local map = wl.Game().map
   local sf = map.player_slots[player.number].starting_field
   if shared_in_start then
      sf = shared_in_start
   else
      player:allow_workers("all")
   end

   local fields = find_ocean_fields(3)
   -- items per expedition (incl. builder): 25
   local items = {
      {
         granite = 1,
         brick = 3,
         reed = 5,
         log = 4,
         frisians_soldier = 1,
         frisians_brickmaker = 1,
         frisians_reed_farmer = 1,
      },
      {
         granite = 1,
         brick = 3,
         reed = 3,
         log = 2,
         frisians_soldier = 1,
         frisians_stonemason = 2,
         frisians_woodcutter = 3,
         frisians_forester = 2,
      },
      {
         iron = 2,
         reed = 2,
         brick = 3,
         frisians_claydigger = 1,
         frisians_soldier = 1,
         frisians_geologist = 1,
         frisians_miner = 2,
         frisians_smelter = 2,
         frisians_blacksmith = 1,
         frisians_smoker = 1,
         frisians_fisher = 1,
      },
   }
   for i,f in pairs(fields) do
      local ship = player:place_ship(f)
      if i == 1 then ship.capacity = 41 else ship.capacity = 42 end
      ship:make_expedition(items[i])
   end
   scroll_to_field(fields[1])
end
}

return init
