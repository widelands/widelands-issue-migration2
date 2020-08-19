world:new_terrain_type{
   name = "winter_mountain3",
   descname = _ "Mountain 3",
   editor_category = "winter",
   is = "mineable",
   valid_resources = { "resource_coal", "resource_iron", "resource_gold", "resource_stones" },
   default_resource = "",
   default_resource_amount = 0,
   textures = { path.dirname(__file__) .. "idle.png" },
   dither_layer = 100,
   temperature = 20,
   humidity = 300,
   fertility = 50,

   enhancement = "winter_forested_mountain2"
}
