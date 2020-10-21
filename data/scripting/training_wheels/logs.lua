-- Detect the player tribe's log producer and teach how to build it, along with Ctrl-click for the roads

include "scripting/coroutine.lua"
include "scripting/messages.lua"
include "scripting/richtext_scenarios.lua"
include "scripting/ui.lua"
include "scripting/training_wheels/utils/buildings.lua"
include "scripting/training_wheels/utils/lock.lua"
include "scripting/training_wheels/utils/ui.lua"

local training_wheel_name = training_wheel_name_from_filename(__file__)

run(function()
   sleep(10)

   local mapview = wl.ui.MapView()
   local interactive_player_slot = wl.Game().interactive_player
   local player = wl.Game().players[interactive_player_slot]
   local tribe = player.tribe

   -- Find the tree collector / log producer building
   local buildings = tribe.buildings
   local log_producer = find_immovable_collector_for_ware(buildings, "tree", "log")
   if log_producer == nil then
      print("Log producer not found for tribe '" .. tribe.name .. "'. Aborting training wheel 'logs'.")
      return
   end

   if #player:get_buildings(log_producer.name) > 0 then
      -- The player already knows how to to this, so don't bother them
      player:mark_training_wheel_as_solved(training_wheel_name)
      return
   end

   -- Find a suitable buildable field close to the the starting field
   local conquering_field = wl.Game().map.player_slots[interactive_player_slot].starting_field
   local conquering_immovable = conquering_field.immovable

   -- Wait for a warehouse
   local warehouse_immovable = wait_for_warehouse(player, buildings)

   if conquering_immovable == nil then
      conquering_immovable = warehouse_immovable
      conquering_field = warehouse_immovable.fields[1]
   end

   local auto_roadbuilding = mapview.auto_roadbuilding_mode

   -- All set - now wait for lock
   wait_for_lock(player, training_wheel_name)

   -- Define our messages
   push_textdomain("training_wheels")

   local size_description = _"Click on a small, medium or big building space, then select the building from the small buildings tab."
   if log_producer.size == "medium" then
      size_description = _"Click on a medium or big building space, then select the building from the medium buildings tab."
   elseif log_producer.size == "big" then
      size_description = _"Click on a big building space, then select the building from the big buildings tab."
   end

   local explain_control_key = join_sentences(_"If you hold down the ‘Ctrl’ key while clicking on the second flag, this will also place more flags on your road if possible.", _"Your carriers can transport your wares faster if they share the load.")

   local msg_logs = {
      title = _"Logs",
      position = "topright",
      body = (
         p(_"You need to supply your tribe with logs. Please build the following building:") ..
         li_object(log_producer.name, log_producer.descname, player.color) ..
         li_image("images/wui/overlays/" .. log_producer.size .. ".png", size_description)
      ),
      h = 280,
      w = 260,
      modal = false
   }

   local msg_enter_roadbuilding = {
      title = _"Roads",
      position = "topright",
      body = (
         li_object(log_producer.name, "Click on the flag in front of the building to start placing a road.", player.color)
      ),
      h = 120,
      w = 260,
      modal = false
   }

   local msg_click_roadbutton = {
      title = _"Roads",
      position = "topright",
      body = (
         li_image("images/wui/fieldaction/menu_build_way.png", "Click on the ‘Build road’ button, then and click on the indicated flag.") ..
         li_arrow(explain_control_key)
      ),
      h = 240,
      w = 260,
      modal = false
   }

   local msg_click_road_endflag = {
      title = _"Roads",
      position = "topright",
      body = (
         li_object(log_producer.name, _"Click on the building’s button…", player.color) ..
         -- We can't get the tribe's flag image, so we settle for the main building
         li_object(warehouse_immovable.descr.name, _"…then click on the flag in front of the target building.", player.color) ..
         li_arrow(explain_control_key)
      ),
      h = 380,
      w = 260,
      modal = false
   }

   local msg_road_not_connected = {
      title = _"Roads",
      position = "topright",
      body = (
         li_image("images/wui/fieldaction/menu_build_way.png", _"Click on the flag in front of the building, then on the ‘Build road’ button, then click on the indicated flag.") ..
         li_arrow(explain_control_key)
      ),
      h = 240,
      w = 260,
      modal = false
   }

   local msg_finished = {
      title = _"Logs",
      position = "topright",
      body = (
         li_object(log_producer.name, _"Well done! We can produce logs now, which we will need for building more buildings.", player.color)
      ),
      h = 140,
      w = 260,
      scroll_back = true,
   }

   pop_textdomain()

   -- Check whether we already have a constructionsite from savegame
   local starting_conquer_range = wl.Game():get_building_description(conquering_immovable.descr.name).conquers
   local constructionsite_search_area = conquering_field:region(starting_conquer_range)
   local constructionsite_field = find_constructionsite_field(log_producer.name, constructionsite_search_area)
   local target_field = nil

   -- If there is no constructionsite from savegame, so we teach player how to place the building
   if constructionsite_field == nil then
      -- Find a suitable field close to a tree. Put some distance between it and the warehouse so that we can get some flags on the road.
      repeat
         target_field = find_immovable_field(conquering_field, "tree", math.ceil(starting_conquer_range / 2), starting_conquer_range + log_producer.workarea_radius / 2)
         target_field = find_buildable_field(target_field, player, log_producer.size, 1, log_producer.workarea_radius / 2)
         if target_field == nil then
            sleep(1000)
         end
      until target_field ~= nil

      target_field:indicate(true)
      campaign_message_box(msg_logs)
      scroll_to_field(target_field)

      -- Wait for player to activate the small building tab
      wait_for_field_action_tab("small")
      mapview.windows.field_action.tabs["small"]:indicate(true)
      while not mapview.windows.field_action.tabs["small"].active do
         sleep(100)
         if not mapview.windows.field_action then
            mapview:indicate(false)
         end
         wait_for_field_action_tab("small")
         mapview.windows.field_action.tabs["small"]:indicate(true)
      end

      -- Explain road building before the road building mode blocks us
      if auto_roadbuilding then
         close_story_messagebox()
         target_field:indicate(true)
         campaign_message_box(msg_click_road_endflag)
         while mapview.windows.field_action do sleep(100) end
         mapview:indicate(false)
         scroll_to_field(target_field)
      end

      -- Now wait for the constructionsite
      constructionsite_field = wait_for_constructionsite_field(log_producer.name, constructionsite_search_area)
      target_field:indicate(false)
   end

   -- When not auto roadbuilding, we need to click on the constructionsite's flag too
   if not mapview.is_building_road then
      mapview:indicate(false)
      close_story_messagebox()

      target_field = constructionsite_field.immovable.flag.fields[1]
      target_field:indicate(true)
      campaign_message_box(msg_enter_roadbuilding)

      -- Wait for player to click a flag
      while not mapview.windows.field_action or not mapview.windows.field_action.buttons["build_road"] do
         while not mapview.windows.field_action or not mapview.windows.field_action.tabs["roads"] do sleep(100) end
         if not mapview.windows.field_action.tabs["roads"].active or mapview.windows.field_action.buttons["build_flag"] then
            while mapview.windows.field_action and (not mapview.windows.field_action.tabs["roads"].active or mapview.windows.field_action.buttons["build_flag"]) do
               sleep(100)
            end
         end
      end
      close_story_messagebox()

      -- Explain road building button
      target_field:indicate(false)
      local build_road_button = mapview.windows.field_action.buttons["build_road"]
      build_road_button:indicate(true)
      campaign_message_box(msg_click_roadbutton)
      while not mapview.is_building_road do sleep(100) end
      mapview:indicate(false)
   end

   -- Indicate target flag for road building and wait for the road
   target_field = warehouse_immovable.flag.fields[1]
   target_field:indicate(true)
   scroll_to_field(target_field)

   while mapview.is_building_road do sleep(100) end
   close_story_messagebox()
   target_field:indicate(false)

   -- Wait for the builder to arrive
   local buildername = player.tribe.builder
   local builder_present = false
   local counter = 0
   repeat
      counter = counter + 1
      if counter % 60 == 0 then
         -- Builder has not arrived, explain road building again and wait for it
         target_field:indicate(true)
         close_story_messagebox()
         campaign_message_box(msg_road_not_connected)
         while not mapview.is_building_road do sleep(100) end
         while mapview.is_building_road do sleep(100) end
         mapview:indicate(false)
      end
      sleep(1000)
      for b_idx, bob in ipairs(constructionsite_field.bobs) do
         if bob.descr.name == buildername then
            builder_present = true
            target_field:indicate(false)
            close_story_messagebox()
            break
         end
      end
   until builder_present == true

   -- Teaching is done, so mark it as solved
   player:mark_training_wheel_as_solved(training_wheel_name)

   -- Wait for the building and congratulate the player

   while #player:get_buildings(log_producer.name) < 1 do sleep(300) end
   msg_finished.field = constructionsite_field
   campaign_message_box(msg_finished)
end)
