terrain_affinity = {
   preferred_temperature = 90,
   preferred_humidity = 600,
   preferred_fertility = 650,
   pickiness = 35,
}

terrain_affinity_black = {
   preferred_temperature = 100,
   preferred_humidity = 150,
   preferred_fertility = 850,
   pickiness = 50,
}

terrain_affinity_desert = {
   preferred_temperature = 140,
   preferred_humidity = 500,
   preferred_fertility = 500,
   pickiness = 50,
}

terrain_affinity_winter = {
   preferred_temperature = 45,
   preferred_humidity = 750,
   preferred_fertility = 400,
   pickiness = 50,
}

spritesheet_sapling = { idle = {
   basename = "rubber_sapling",
   hotspot = {8, 18},
   fps = 2,
   frames = 4,
   columns = 2,
   rows = 2
}}

spritesheet_pole = { idle = {
   basename = "rubber_pole",
   hotspot = {11, 44},
   fps = 2,
   frames = 4,
   columns = 2,
   rows = 2
}}

spritesheet_mature = { idle = {
   basename = "rubber_mature",
   hotspot = {18, 75},
   fps = 2,
   frames = 4,
   columns = 2,
   rows = 2
}}

spritesheet_old = {
   idle = {
      basename = "rubber_old",
      hotspot = {18, 75},
      fps = 2,
      frames = 4,
      columns = 2,
      rows = 2
   },
   fall = {
      basename = "rubber_fall",
      hotspot = {19, 80},
      fps = 10,
      frames = 10,
      columns = 5,
      rows = 2,
      play_once = true
   }
}
