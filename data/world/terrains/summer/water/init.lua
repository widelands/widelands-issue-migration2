world:new_terrain_type{
   name = "summer_water",
   descname = _ "Water",
   is = "water",
   valid_resources = {"resource_fish"},
   default_resource = "resource_fish",
   default_resource_amount = 4,
   textures = path.list_files(path.dirname(__file__) .. "water_??.png"),
   dither_layer = 180,
   fps = 14,
   temperature = 100,
   humidity = 999,
   fertility = 1,
}
