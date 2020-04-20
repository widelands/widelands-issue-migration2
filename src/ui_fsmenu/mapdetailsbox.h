#ifndef WL_UI_FSMENU_MAPDETAILSBOX_H
#define WL_UI_FSMENU_MAPDETAILSBOX_H
#include "ui_basic/box.h"
#include "ui_basic/button.h"
#include "ui_basic/multilinetextarea.h"
#include "ui_basic/textarea.h"

struct GameSettingsProvider;
class MapDetailsBox : public UI::Box {
public:
	MapDetailsBox(Panel* parent,
	              int32_t x,
	              int32_t y,
	              int32_t padding,
	              int32_t indent,
	              int32_t max_x = 0,
	              int32_t max_y = 0);
	~MapDetailsBox();

	void update(GameSettingsProvider* settings);

	// irrg I do not want to do this!!!
	UI::Button& select_map_button();

private:
	UI::Box title_box_;

	UI::Textarea map_name_;
	UI::Button select_map_;
	UI::MultilineTextarea map_description_;

	void load_map_info(GameSettingsProvider* settings);
};

#endif  // WL_UI_FSMENU_MAPDETAILSBOX_H
