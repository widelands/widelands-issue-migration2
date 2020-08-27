-- ===================================
--    Campaign Specific Data
-- ===================================

return {
   barbarians = {
      buildings = {
         "barbarians_market",
         "barbarians_custom_building"
      },
      workers_order = {
         {
            {name = "barbarians_custom_worker" }
         },
      },
      wares_order = {
         {
            {
               name= "custom_ware",
               default_target_quantity = 500,
               preciousness = 14
            }
         },
      }
   }
}
