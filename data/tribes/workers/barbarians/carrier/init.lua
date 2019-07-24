dirname = path.dirname(__file__)

animations = {
   idle = {
      directory = dirname,
      basename = "idle",
      fps = 5,
      frames = 100,
      columns = 10,
      rows = 10,
      hotspot = { 14, 21 }
   },
   walk = {
      directory = path.dirname(__file__),
      basename = "walk",
      fps = 10,
      frames = 10,
      columns = 3,
      rows = 4,
      hotspot = { 9, 19 },
      directional = true
   },
   walkload = {
      directory = path.dirname(__file__),
      basename = "walkload",
      fps = 10,
      frames = 10,
      columns = 3,
      rows = 4,
      directional = true,
      hotspot = { 7, 22 }
   }
}

tribes:new_carrier_type {
   msgctxt = "barbarians_worker",
   name = "barbarians_carrier",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("barbarians_worker", "Carrier"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {}, -- This will give the worker the property "buildable"

   animations = animations,
}
