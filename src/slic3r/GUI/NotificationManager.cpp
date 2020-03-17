#include "NotificationManager.hpp"

#include "GUI_App.hpp"
#include "GLCanvas3D.hpp"

#include <wx/glcanvas.h>
#include <iostream>

#define NOTIFICATION_UP_TIME  10
#define NOTIFICATION_MAX_MOVE 3.0f

namespace Slic3r {
namespace GUI {

//------PopNotification--------
NotificationManager::PopNotification::PopNotification(const NotificationData& n) :
	  m_data          (n)
	, m_creation_time (wxGetLocalTime())
{
}
//NotificationManager::PopNotification::~PopNotification()
//{}
NotificationManager::PopNotification::RenderResult NotificationManager::PopNotification::render(GLCanvas3D& canvas, const float& initial_x)
{
	if (m_finished)
		return RenderResult::Finished;
	
	if (wxGetLocalTime() - m_creation_time >= m_data.duration)
		m_close_pending = true;

	if (m_close_pending) {
		m_finished = true;
		//canvas.request_extra_frame();
		return RenderResult::ClosePending;
	}

	RenderResult ret_val = RenderResult::Static;
	Size cnv_size = canvas.get_canvas_size();
	ImGuiWrapper& imgui = *wxGetApp().imgui();
	imgui.set_next_window_pos(1.0f * (float)cnv_size.get_width(), 1.0f * (float)cnv_size.get_height() - m_current_x, ImGuiCond_Always, 1.0f, 0.0f);

	bool new_target = false;
	if(m_target_x != initial_x + m_window_height)
	{
		m_target_x = initial_x + m_window_height;
		new_target = true;
	}
	
	if (m_current_x < m_target_x) {
		if (new_target || m_move_step < 1.0f)
			m_move_step = std::min((m_target_x - m_current_x) / 20, NOTIFICATION_MAX_MOVE);
		m_current_x += m_move_step;
		ret_val = RenderResult::Moving;
	}
	if (m_current_x > m_target_x)
		m_current_x = m_target_x;

	bool shown = true;
	std::string name;
	for (size_t i = 0; i < m_data.id; i++)
		name += " ";
	//if (imgui.begin(_(L("Notification")), &shown, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse)) {
	if (imgui.begin(name, &shown, ImGuiWindowFlags_NoCollapse)) {
		if (shown) {
			ImVec2 win_size = ImGui::GetWindowSize();
			m_window_height = win_size.y;
			m_window_width = win_size.x;
			
			const ImVec4& color = ImGui::GetStyleColorVec4(ImGuiCol_Separator);
			ImGui::PushStyleColor(ImGuiCol_Text, color);
			imgui.text(m_data.text.c_str());
			ImGui::PopStyleColor();
			
		} else {
			// the user clicked on the [X] button
			m_close_pending = true;
			canvas.set_as_dirty();
		}
	}

	imgui.end();
	return ret_val;
}

float NotificationManager::PopNotification::get_top() const 
{
	return m_target_x;
}

void NotificationManager::PopNotification::update()
{
	m_creation_time = wxGetLocalTime();
}
//------NotificationManager--------
NotificationManager::NotificationManager()
{
}
NotificationManager::~NotificationManager()
{
	for (PopNotification* notification : m_pop_notifications)
	{
		delete notification;
	}
}
void NotificationManager::push_notification(const NotificationType type, GLCanvas3D& canvas)
{
	std::string text;
	switch (type) {
	case NotificationType::SlicingComplete: text = "Slicing finished"; break;
	default:                                return;
	}
	push_notification(type, text, canvas);
}
void NotificationManager::push_notification(const std::string& text, GLCanvas3D& canvas)
{
	push_notification(NotificationType::Undefined, text, canvas);
}
void NotificationManager::push_notification(const NotificationType type, const std::string& text, GLCanvas3D& canvas)
{
	if (!this->find_older(type))
		m_pop_notifications.emplace_back(new PopNotification({ type, text, NOTIFICATION_UP_TIME, m_next_id++ }));
	else
		m_pop_notifications.back()->update();
	std::cout << "PUSH: " << text << std::endl;
	canvas.render();
}
void NotificationManager::render_notifications(GLCanvas3D& canvas)
{
	float last_x = 0.0f;
	bool request_next_frame = false;
	for (auto it = m_pop_notifications.begin(); it != m_pop_notifications.end();) {
		if ((*it)->get_finished()) {
			delete (*it);
			it = m_pop_notifications.erase(it);
		} else {
			PopNotification::RenderResult res = (*it)->render(canvas, last_x);
			if (res != PopNotification::RenderResult::Finished)
				last_x = (*it)->get_top();
			if (res == PopNotification::RenderResult::Moving || res == PopNotification::RenderResult::ClosePending)
				request_next_frame = true;
			++it;
		}
	}
	if(request_next_frame)
	{
		canvas.request_extra_frame();
	}
}

bool NotificationManager::find_older(NotificationType type)
{
	for (auto it = m_pop_notifications.begin(); it != m_pop_notifications.end(); ++it)
	{
		if((*it)->get_type() == type && !(*it)->get_finished() && it != m_pop_notifications.end()-1)
		{
			std::rotate(it, it+1 , m_pop_notifications.end());
			return true;
		}
	}
	return false;
}
void NotificationManager::print_to_console() const 
{
	/*
	std::cout << "---Notifications---" << std::endl;
	for (const Notification &notification :m_notification_container) {
		std::cout << "Notification " << ": " << notification.data.text << std::endl;
	}
	*/
}

}//namespace GUI
}//namespace Slic3r