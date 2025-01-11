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
            -- the longest possible transition from tiny to ripe field:
            --   2 * (30 + 45 + 50) = 250 sec
            --   see data/tribes/immovables/wheatfield/ and src/logic/map_objects/immovable_program.cc - ImmovableProgram::ActAnimate::execute()
            -- how many fields are needed for 100% productivity in worst case (nearest fields, longest field transition):
            --   1 + (250 - (6 + 2 * 1.8 + 10)) / 54.4 = 5.235
            -- calculation of productivity with 4 farms:
            --   irwinhall(8, scale=1/8).cdf(4/5.235) = 0.996
            --   using SciPy 1.14+
            -- min. time total (free 4 nearest fields):  29.2 + 25.2 = 54.4 sec
            -- max. time total (free 4 furthest fields): 40   + 36   = 76   sec
            "call=plant",
            "call=harvest",
         }
      },
      plant = {
         -- TRANSLATORS: Completed/Skipped/Did not start planting wheat because ...
         descname = _("planting wheat"),
         actions = {
            -- time of worker: 19.2-30 sec
            -- min. time: 19.2 + 10 = 29.2 sec
            -- max. time: 30   + 10 = 40   sec
            "callworker=plant",
            "animate=working duration:7s",
            "sleep=duration:3s"
         }
      },
      harvest = {
         -- TRANSLATORS: Completed/Skipped/Did not start harvesting wheat because ...
         descname = _("harvesting wheat"),
         actions = {
            -- time of worker: 21.2-32 sec
            -- min. time: 21.2 + 4 = 25.2 sec
            -- max. time: 32   + 4 = 36   sec
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
