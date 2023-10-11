include "test/scripting/check_game_end.lua"

map = game.map
atl = game.players[2]

function f(x, y)
  r = map:get_field(x, y)
  return r
end

run(function()
  sleep(2000)
  print("Place a port for the winner to conquer some land.")
  atl:place_building("atlanteans_port", f(9, 141), false, true)
  -- Time limited win condition, let's just wait.
end)

check_win_condition(2)
