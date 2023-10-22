push_textdomain("tribes")

local dirname = path.dirname (__file__)

wl.Descriptions():new_carrier_type {
   name = "amazons_carrier",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext ("amazons_worker", "Carrier"),
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {}, -- This will give the worker the property "buildable"

   programs = {
      fetch_water = {
         -- first bucket takes 1 sec, second bucket (from Water Gatherer’s Hut) takes 3.6 sec, so 2 buckets take 4.6 sec
         -- steps from building to water: 2-11, min+max average 6.5
         -- min. worker time: 2 * 2 * 1.8 + 4.6 = 11.8 sec
         -- max. worker time: 2 * 11 * 1.8 + 4.6 = 44.2 sec
         -- avg. worker time: 2 * 6.5 * 1.8 + 4.6 = 28 sec
         "findspace=size:swim radius:8",
         "walk=coords",
         "animate=fetch_water duration:1s",
         "createware=water",
         "return"
      }
   },

   animation_directory = dirname,
   ware_hotspot = {0, 29},
   spritesheets = {
      idle = {
         hotspot = {11, 29},
         fps = 10,
         frames = 80,
         columns = 10,
         rows = 8
      },
      fetch_water = {
         hotspot = {10, 22},
         fps = 10,
         frames = 10,
         columns = 5,
         rows = 2
      },
      walk = {
         directional = true,
         hotspot = {17, 28},
         fps = 15,
         frames = 30,
         columns = 6,
         rows = 5
      },
      walkload = {
         directional = true,
         hotspot = {17, 31},
         fps = 15,
         frames = 30,
         columns = 6,
         rows = 5
      },
   },
}

pop_textdomain()
