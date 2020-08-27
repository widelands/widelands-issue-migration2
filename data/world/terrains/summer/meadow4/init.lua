push_textdomain("world")

wl.World():new_terrain_type{
   name = "summer_meadow4",
   descname = _ "Meadow 4",
   is = "arable",
   tooltips = {
      -- TRANSLATORS: This is an entry in a terrain tooltip. Try to use 1 word if possible.
      _"likes trees",
   },
   valid_resources = {"resource_water"},
   default_resource = "resource_water",
   default_resource_amount = 10,
   textures = { path.dirname(__file__) .. "idle.png" },
   dither_layer = 350,
   temperature = 110,
   humidity = 650,
   fertility = 750,
}

pop_textdomain()
