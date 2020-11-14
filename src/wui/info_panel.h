/*
 * Copyright (C) 2020 by the Widelands Development Team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */
#ifndef WL_WUI_INFO_PANEL_H
#define WL_WUI_INFO_PANEL_H

#include <list>
#include <memory>

#include "logic/message_id.h"
#include "ui_basic/dropdown.h"
#include "ui_basic/textarea.h"
#include "wui/logmessage.h"

class InfoPanel;
class InteractiveBase;
class MainToolbar;

namespace Widelands {
struct Message;
struct MessageQueue;
}

class MessagePreview : public UI::Textarea {
public:
	MessagePreview(InfoPanel&, const Widelands::Message&, Widelands::MessageId);
	MessagePreview(InfoPanel&, const std::string& text, const std::string& tooltip);
	~MessagePreview() override {
	}

	void think() override;
	void draw(RenderTarget&) override;
	bool handle_mousepress(uint8_t, int32_t, int32_t) override;

private:
	InfoPanel& owner_;
	uint32_t creation_time_;

	const Widelands::Message* message_;
	Widelands::MessageId id_;
};

class InfoPanel : public UI::Panel {
public:
	InfoPanel(InteractiveBase&);
	~InfoPanel() override {
	}

	// Update the text area without relayouting
	void set_time_string(const std::string&);
	void set_speed_string(const std::string&);
	void set_fps_string(const std::string&);
	void set_coords_string(const std::string&);

	void log_message(const std::string&);

	void think() override;
	void layout() override;

	void set_toolbar(MainToolbar&);

	void draw(RenderTarget&) override;
	bool handle_mousepress(uint8_t, int32_t, int32_t) override;
	bool handle_mouserelease(uint8_t, int32_t, int32_t) override;
	bool handle_mousemove(uint8_t, int32_t, int32_t, int32_t, int32_t) override;

private:
	friend class MessagePreview;

	InteractiveBase& ibase_;

	bool on_top_;

	enum class DisplayMode { kPinned, kMinimized, kOnMouse_Visible, kOnMouse_Hidden };
	DisplayMode display_mode_;
	void toggle_mode();

	Vector2i last_mouse_pos_;
	bool is_mouse_over_panel() const;
	void set_textareas_visibility(bool);

	void update_time_speed_string();

	MainToolbar* toolbar_;

	UI::Dropdown<DisplayMode> toggle_mode_;
	UI::Textarea text_time_speed_, text_fps_, text_coords_;

	std::list<MessagePreview*> messages_;
	size_t index_of(const MessagePreview&) const;
	void pop_message(MessagePreview&);
	void push_message(MessagePreview&);
	std::unique_ptr<Notifications::Subscriber<LogMessage>> log_message_subscriber_;
	const Widelands::MessageQueue* message_queue_;
	std::unique_ptr<Widelands::MessageId> last_message_id_;

	bool draw_real_time_;
	std::string time_string_, speed_string_;
};

#endif  // end of include guard: WL_WUI_INFO_PANEL_H
