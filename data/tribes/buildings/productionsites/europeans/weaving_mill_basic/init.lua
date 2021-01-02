push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_productionsite_type {
   name = "europeans_weaving_mill_basic",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("europeans_building", "Basic Weaving Mill"),
   icon = dirname .. "menu.png",
   size = "big",

   enhancement = {
        name = "europeans_weaving_mill_normal",
        enhancement_cost = {
          brick = 3,
          grout = 3,
          spidercloth = 3,
          quartz = 1
        },
        enhancement_return_on_dismantle = {
          granite = 3,
          quartz = 1,
        },
   },

   buildcost = {
      planks = 4,
      reed = 4,
      granite = 2
   },
   return_on_dismantle = {
      log = 4,
      granite = 2
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 36, 74 },
      },
      build = {
         pictures = path.list_files(dirname .. "build_??.png"),
         hotspot = { 36, 74 },
      },
      working = {
         pictures = path.list_files(dirname .. "working_??.png"),
         hotspot = { 36, 74 },
      },
   },

   aihints = {
      prohibited_till = 3600,
      supports_seafaring = true
   },

   working_positions = {
      europeans_weaver_basic = 1
   },

   inputs = {
      { name = "spider_silk", amount = 8 }
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _"working",
         actions = {
            "call=produce_spidercloth_basic",
            "call=produce_spidercloth"
         }
      },
      produce_spidercloth_basic = {
         -- TRANSLATORS: Completed/Skipped/Did not start weaving because ...
         descname = _"weaving",
         actions = {
            "return=skipped when economy needs spidercloth",
            "sleep=duration:60s",
            "consume=spider_silk",
            "playsound=sound/barbarians/weaver priority:90%",
            "animate=working duration:60s",
            "produce=spidercloth"
         }
      },
      produce_spidercloth = {
         -- TRANSLATORS: Completed/Skipped/Did not start weaving because ...
         descname = _"weaving",
         actions = {
            "return=skipped unless economy needs spidercloth",
            "sleep=duration:50s",
            "consume=spider_silk:3",
            "playsound=sound/barbarians/weaver priority:90%",
            "animate=working duration:50s",
            "produce=spidercloth:3"
         }
      },
   },
}

pop_textdomain()
