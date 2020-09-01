push_textdomain("world")

wl.World():new_terrain_type{
   name = "summer_meadow1",
   descname = _ "Meadow 1",
   is = "arable",
   tooltips = {
      -- TRANSLATORS: This is an entry in a terrain tooltip. Try to use 1 word if possible.
      _"likes trees",
   },

   valid_resources = {"resource_water"},
   default_resource = "resource_water",
   default_resource_amount = 10,

   textures = { path.dirname(__file__) .. "idle.png" },

   dither_layer = 340,

   temperature = 100,
   humidity = 600,
   fertility = 700,
}

pop_textdomain()
