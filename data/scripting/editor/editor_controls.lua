include "scripting/richtext.lua"
include "txts/help/common_helptexts.lua"

local toggle_minimap_hotkey = help_toggle_minimap_hotkey()
local toggle_building_spaces_hotkey = help_toggle_building_spaces_hotkey()
local screenshot_hotkey = help_screenshot_hotkey()
local toggle_fullscreen_hotkey = help_toggle_fullscreen_hotkey()
local open_debug_console_hotkey = help_open_debug_console_hotkey()

push_textdomain("widelands_editor")

local r = {
   title = _("Controls"),
   text =
      h2(_("Keyboard Shortcuts")) ..
      p(
         -- TRANSLATORS: This is the helptext for an access key combination.
         dl(help_format_hotkey(wl.ui.get_shortcut("encyclopedia")), _("Help")) ..
         -- TRANSLATORS: This is the helptext for an access key combination.
         dl(help_format_hotkey(wl.ui.get_shortcut("editor_tools")), _("Toggle tools menu")) ..
         toggle_minimap_hotkey ..
         toggle_building_spaces_hotkey ..
         -- TRANSLATORS: This is the helptext for an access key combination.
         dl(help_format_hotkey(wl.ui.get_shortcut("editor_players")), _("Toggle player menu")) ..
         -- TRANSLATORS: This is the helptext for an access key combination.
         dl(help_format_hotkey(wl.ui.get_shortcut("editor_undo")), _("Undo")) ..
         -- TRANSLATORS: This is the helptext for an access key combination.
         dl(help_format_hotkey(wl.ui.get_shortcut("editor_redo")), _("Redo")) ..
         -- TRANSLATORS: This is the helptext for an access key combination.
         dl(help_format_hotkey(wl.ui.get_shortcut("editor_info")), _("Activate information tool")) ..
         -- TRANSLATORS: This is the helptext for an access key combination.
         dl(help_format_hotkey(wl.ui.get_shortcut("load")), _("Load map")) ..
         -- TRANSLATORS: This is the helptext for an access key combination.
         dl(help_format_hotkey(wl.ui.get_shortcut("save")), _("Save map")) ..
         screenshot_hotkey ..
         toggle_fullscreen_hotkey ..
         open_debug_console_hotkey
      ) ..

      h2(_("Tools")) ..
      p(
         -- TRANSLATORS: This is the helptext for an access key combination.
         dl(help_format_hotkey(help_editor_toolsize_tips()), _("Change tool size")) ..
         -- TRANSLATORS: This is an access key combination.
         dl(help_format_hotkey(pgettext("hotkey", "Click")),
                               -- TRANSLATORS: This is the helptext for an access key combination.
                               _("Place new elements on the map, or increase map elements by the value selected by ‘Increase/Decrease value’")) ..
         -- TRANSLATORS: This is an access key combination.
         dl(help_format_hotkey(pgettext("hotkey", "Shift + Click")),
                               -- TRANSLATORS: This is the helptext for an access key combination.
                               _("Remove elements from the map, or decrease map elements by the value selected by ‘Increase/Decrease value’")) ..
         -- TRANSLATORS: This is an access key combination.
         dl(help_format_hotkey(pgettext("hotkey", "Ctrl + Click")),
                               -- TRANSLATORS: This is the helptext for an access key combination.
                               _("Set map elements to the value selected by ‘Set Value’"))
      )

..
vspace(20)
..
wl.ui.get_editor_shortcut_help()

}
pop_textdomain()
return r
