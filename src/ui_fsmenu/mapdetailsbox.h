#ifndef WL_UI_FSMENU_MAPDETAILSBOX_H
#define WL_UI_FSMENU_MAPDETAILSBOX_H
#include "ui_basic/box.h"
#include "ui_basic/button.h"
#include "ui_basic/multilinetextarea.h"
#include "ui_basic/textarea.h"
#include <logic/map.h>

struct GameSettingsProvider;
class MapDetailsBox : public UI::Box {
public:
	MapDetailsBox(Panel* parent,
	              uint32_t standard_element_width,
	              uint32_t standard_element_height,
	              int32_t max_x = 0,
	              int32_t max_y = 0);
	~MapDetailsBox();

	void update(GameSettingsProvider* settings, Widelands::Map& map);

	/// passed callback is called when the select map button is clicked
	void set_select_map_action(std::function<void()> action);

	void force_new_dimensions(float scale, uint32_t standard_element_height, uint32_t i);

	// TODO(jmoerschbach): only used by multiplayer screen...
	void set_map_description_text(const std::string& text);

private:
	UI::Textarea title_;
	UI::Box title_box_;
	UI::Textarea map_name_;
	UI::Button select_map_;
	UI::MultilineTextarea map_description_;

	void show_map_description(Widelands::Map& map, GameSettingsProvider* settings);
};

#endif  // WL_UI_FSMENU_MAPDETAILSBOX_H
