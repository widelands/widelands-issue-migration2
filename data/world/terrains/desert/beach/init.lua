-- RST
-- .. _lua_world_terrains:
--
-- Terrains
-- --------
--
-- Terrains define the basic look of the map.
--
-- .. code-block:: none
--
--         *-------*
--        / \     / \
--       /   \   /   \
--      /     \ /     \
--     *-------*-------*
--      \     / \     /
--       \   /   \   /
--        \ /     \ /
--         *-------*
--
-- Terrain tiles have a triangular shape, and 6 of them will be combined to form a hexagon. Each vertex between the terrains (* in the figure) will form a node that is influenced by the 6 terrains surrounding it, and where other map entities can be placed. You can find more information on the terrains' shape and on how to create textures on the `wiki <https://www.widelands.org/wiki/HelpTerrains/>`_.
--
-- Each terrain tile will also influence some properties for the map entities that are placed on its 3 vertices, like:
--
-- * Which resources can go on a map node.
-- * How well which type of tree will grow on a map node.
-- * Which map objects can be built on the nodes or move through them.
--
-- .. function:: new_terrain_type{table}
--
--    This function adds the definition of a terrain to the engine.
--
--    :arg table: This table contains all the data that the game engine will add
--       to this terrain. It contains the following entries:
--
--    **name**
--        *Mandatory*. A string containing the internal name of this terrain, e.g.::
--
--            name = "summer_meadow1",
--
--    **descname**
--        *Mandatory*. The translatable display name, e.g.::
--
--            descname = _"Meadow 1",
--
--    **editor_category**
--        *Deprecated*. The category that is used in the editor tools for placing a
--        terrain of this type on the map, e.g.::
--
--            editor_category = "summer",
--
--    **is**
--        *Mandatory*. The type of this terrain, which determines if the nodes
--        surrounding the terrain will be walkable or navigable, if mines or buildings
--        can be built on them, if flags can be built on them, and so on.
--        The following properties are available:
--
--         * ``arable``: Allows building of normal buildings and roads.
--         * ``mineable``: Allows building of mines and roads.
--         * ``walkable``: Allows building of flags and roads only.
--         * ``water``: Nothing can be built here, but ships and aquatic animals can pass.
--         * ``unreachable``: Nothing can be built here, and nothing can walk on it,
--           and nothing will grow.
--         * ``unwalkable``: Nothing can be built here, and nothing can walk on it.
--
--        Example::
--
--           is = "arable",
--
--        *Note: There is currently some interdependency between ``is`` and
--        ``valid_resources``, so not all combinations are possible. See*
--        `Issue #2038 <https://github.com/widelands/widelands/issues/2038>`_
--        *for more information.*
--
--    **tooltips**
--        *Deprecated*. Additional custom tooltip entries, e.g.::
--
--            tooltips = {
--               _"likes trees",
--            },
--
--    **valid_resources**
--        *Mandatory*. The list of mineable resources that can be found on this terrain.
--        Leave this empty (``{}``) if you want no resources on this terrain. Example::
--
--            valid_resources = {"resource_water"},
--
--        *Note: There is currently some interdependency between ``is`` and
--        ``valid_resources``, so not all combinations are possible. See*
--        `Issue #2038 <https://github.com/widelands/widelands/issues/2038>`_
--        *for more information.*
--
--    **default_resource**
--        *Mandatory*. A resource type that can always be found on this terrain when
--        a new game is started, unless the map maker places some resources there via
--        the editor. Use the empty string
--        (``""``) if you want no default resource. Example::
--
--            default_resource = "resource_water",
--
--    **default_resource_amount**
--        *Mandatory*. The amount of the above default resource that will
--        automatically be placed on this terrain, e.g.::
--
--            default_resource_amount = 10,
--
--    **textures**
--        *Mandatory*. The images used for this terrain. Examples::
--
--            textures = { path.dirname(__file__) .. "idle.png" }, - A static terrain
--            textures = path.list_files(path.dirname(__file__) .. "lava_??.png"), -- An animated terrain
--
--    **dither_layer**
--        *Mandatory*. Terrains will be blended slightly over each other in order
--        to hide the harsh edges of the triangles. This describes the
--        `z layer <https://en.wikipedia.org/wiki/Z-order>`_ of a terrain when
--        rendered next to another terrain. Terrains with a higher value will be
--        dithered on top of terrains with a lower value. Example::
--
--            dither_layer = 340,
--
--    **temperature**
--        *Mandatory*. A terrain affinity constant. These are used to model how well
--        trees will grow on this terrain. Temperature is in arbitrary units. Example::
--
--            temperature = 100,
--
--    **humidity**
--        *Mandatory*. A terrain affinity constant. These are used to model how well
--        trees will grow on this terrain. Values range from 1 - 1000 (1000 being very wet).
--        Example::
--
--            humidity = 600,
--
--    **fertility**
--        *Mandatory*. A terrain affinity constant. These are used to model how well
--        trees will grow on this terrain. Values range from 1 - 1000 (1000 being very
--        fertile). Example::
--
--            fertility = 700,
--
--    **enhancement**
--        *Optional*. The terrain this terrain can be turned into by buildings like
--        the amazon gardening center. Example::
--
--            enhancement = "summer_meadow3",
--

wl.World():new_terrain_type{
   name = "desert_beach",
   descname = _ "Beach",
   is = "walkable",
   valid_resources = {},
   default_resource = "",
   default_resource_amount = 0,
   textures = { path.dirname(__file__) .. "idle.png" },
   dither_layer = 60,
   temperature = 179,
   humidity = 500,
   fertility = 100,
}
