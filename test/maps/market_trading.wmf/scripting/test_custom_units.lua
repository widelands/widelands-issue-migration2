run(function()
   sleep(2000)
   prefilled_buildings(p1, { "barbarians_custom_building", 22, 27 })
   custom_building = map:get_field(22, 27).immovable
   connected_road("normal", p1, custom_building.flag, "tr,tl|", true)

   sleep(100000)

   hq = p1:get_buildings("barbarians_headquarters")[1]
   assert_true(hq:get_wares("custom_ware") > 350, "We should have collected some of the custom ware")
   assert_true(barbarians_custom_worker_program_counter   > 0, "The worker program script should have been called")
   assert_true(barbarians_custom_building_program_counter > 0, "The building program script should have been called")

   stable_save(game, "custom_units", 50 * 1000)

   assert_true(hq:get_wares("custom_ware") > 350, "We should have collected some of the custom ware")
   assert_true(barbarians_custom_worker_program_counter   > 0, "The worker program script should have been called")
   assert_true(barbarians_custom_building_program_counter > 0, "The building program script should have been called")

   print("# All Tests passed.")
   wl.ui.MapView():close()
end)
