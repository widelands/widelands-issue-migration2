push_textdomain("tribes_encyclopedia")

include "scripting/richtext.lua"
include "txts/help/common_helptexts.lua"

local r = {
   title = _("Controls"),
   text =
         h2(_("Window Control")) ..
         p(
               -- TRANSLATORS: This is an access key combination.
               dl(help_format_hotkey(pgettext("hotkey", "Right-click")),
                                     -- TRANSLATORS: This is the helptext for an access key combination.
                                     _("Close window")) ..
               -- TRANSLATORS: This is an access key combination.
               dl(help_format_hotkey(pgettext("hotkey", "Middle-click or Ctrl + Left-click")),
                                     -- TRANSLATORS: This is the helptext for an access key combination.
                                     _("Minimize/Maximize window")) ..
               -- TRANSLATORS: This is an access key combination.
               dl(help_format_hotkey(pgettext("hotkey", "Ctrl + Left-click on Button")),
                                     -- TRANSLATORS: This is the helptext for an access key combination.
                                     _("Skip confirmation dialog"))) ..

         h2(_("Table Control")) ..
         h3(_("In tables that allow the selection of multiple entries, the following key combinations are available:")) ..
         p(
               -- TRANSLATORS: This is an access key combination.
               dl(help_format_hotkey(pgettext("hotkey", "Ctrl + Click")),
                                     -- TRANSLATORS: This is the helptext for an access key combination.
                                     pgettext("table_control", "Select multiple entries")) ..
               -- TRANSLATORS: This is an access key combination.
               dl(help_format_hotkey(pgettext("hotkey", "Shift + Click")),
                                     -- TRANSLATORS: This is the helptext for an access key combination.
                                     pgettext("table_control", "Select a range of entries")) ..
               -- TRANSLATORS: This is the helptext for an access key combination.
               dl(help_format_hotkey(wl.ui.get_shortcut("selectall")), pgettext("table_control", "Select all entries"))) ..

         h2(_("Road Control")) ..
         p(
               -- TRANSLATORS: This is an access key combination.
               dl(help_format_hotkey(pgettext("hotkey", "Ctrl + Left-click")),
                                     -- TRANSLATORS: This is the helptext for an access key combination.
                                     _("While connecting two flags: Place flags automatically")) ..
               -- TRANSLATORS: This is an access key combination.
               dl(help_format_hotkey(pgettext("hotkey", "Ctrl + Left-click")),
                                     -- TRANSLATORS: This is the helptext for an access key combination.
                                     _("While removing a flag: Remove all flags up to the first junction"))) ..
         -- Keyboard Shortcuts
         wl.ui.get_ingame_shortcut_help()
}
pop_textdomain()
return r
