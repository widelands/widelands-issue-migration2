wl.World():new_terrain_type{
   name = "wasteland_forested_mountain2",
   descname = _ "Forested Mountain 2",
   is = "mineable",
   tooltips = {
      -- TRANSLATORS: This is an entry in a terrain tooltip. Try to use 1 word if possible.
      _"likes trees",
   },
   valid_resources = {"resource_coal", "resource_iron", "resource_gold", "resource_stones"},
   default_resource = "",
   default_resource_amount = 0,
   textures = { path.dirname(__file__) .. "idle.png" },
   dither_layer = 81,
   temperature = 95,
   humidity = 200,
   fertility = 400,
}
