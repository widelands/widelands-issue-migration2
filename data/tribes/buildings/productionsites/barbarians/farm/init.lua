push_textdomain("tribes")

local dirname = path.dirname(__file__)

wl.Descriptions():new_productionsite_type {
   name = "barbarians_farm",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("barbarians_building", "Farm"),
   icon = dirname .. "menu.png",
   size = "big",

   buildcost = {
      log = 4,
      blackwood = 1,
      granite = 3
   },
   return_on_dismantle = {
      log = 1,
      blackwood = 1,
      granite = 2
   },

   animation_directory = dirname,
   animations = {
      unoccupied = {
         hotspot = { 69, 76 },
      },
   },

   spritesheets = {
      idle = {
         frames = 20,
         rows = 5,
         columns = 4,
         hotspot = { 69, 76 }
      },
      build = {
         frames = 4,
         rows = 2,
         columns = 2,
         hotspot = { 69, 76 }
      },
      working = {
         frames = 20,
         rows = 5,
         columns = 4,
         hotspot = { 69, 76 }
      },
   },

   aihints = {
      space_consumer = true,
      prohibited_till = 400,
      very_weak_ai_limit = 1,
      weak_ai_limit = 3
   },

   working_positions = {
      barbarians_farmer = 1
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _("working"),
         actions = {
            -- longest possible transition from tiny to ripe field:
            --   2 * (30 + 45 + 50) = 250 sec
            --   see data/tribes/immovables/wheatfield/ and src/logic/map_objects/immovable_program.cc - ImmovableProgram::ActAnimate::execute()
            -- how many fields are needed for reliable 100% productivity: 6
            --   with 5 fields, this inequation is not fulfilled:
            --   (250 + 55.84 - (6 + 2.2 * 1.8 + 10)) / 55.84 < 5
            -- min. time total (free 5 nearest fields):  29.92 + 25.92 = 55.84 sec
            -- min. time total (free 6 nearest fields):  30.4  + 26.4  = 56.8  sec
            -- max. time total (free 6 furthest fields): 38.2  + 34.2  = 72.4  sec
            "call=plant",
            "call=harvest",
         }
      },
      plant = {
         -- TRANSLATORS: Completed/Skipped/Did not start planting wheat because ...
         descname = _("planting wheat"),
         actions = {
            -- time of worker: 20.4-28.2 sec, min. time for 5 fields 19.92 sec
            -- min. time (5 fields): 19.92 + 10 = 29.92 sec
            -- min. time:            20.4  + 10 = 30.4  sec
            -- max. time:            28.2  + 10 = 38.2  sec
            "callworker=plant",
            "animate=working duration:7s",
            "sleep=duration:3s"
         }
      },
      harvest = {
         -- TRANSLATORS: Completed/Skipped/Did not start harvesting wheat because ...
         descname = _("harvesting wheat"),
         actions = {
            -- time of worker: 22.4-30.2 sec, min. time for 5 fields 21.92 sec
            -- min. time (5 fields): 21.92 + 4 = 25.92 sec
            -- min. time:            22.4  + 4 = 26.4  sec
            -- max. time:            30.2  + 4 = 34.2  sec
            "callworker=harvest",
            "animate=working duration:3s",
            "sleep=duration:1s"
         }
      },
   },
   out_of_resource_notification = {
      -- Translators: Short for "Out of ..." for a resource
      title = _("No Fields"),
      heading = _("Out of Fields"),
      message = pgettext("barbarians_building", "The farmer working at this farm has no cleared soil to plant his seeds."),
      productivity_threshold = 30
   },
}

pop_textdomain()
