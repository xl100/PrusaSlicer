#include "NotificationManager.hpp"

#include "GUI_App.hpp"
#include "GLCanvas3D.hpp"

#include <wx/glcanvas.h>
#include <iostream>

namespace Slic3r {
namespace GUI {

//------PopNotification--------
NotificationManager::PopNotification::PopNotification(const NotificationData& n) :
	 m_data(n)
	,m_finnished(false)
	,m_closed_by_user(false)
	,m_moving_anchor(0.0f)
	, m_window_height(0.0f)
    , m_window_width(0.0f)
	, m_initial_x(0.0f)
	,m_creation_time(wxGetLocalTime())
{
}
//NotificationManager::PopNotification::~PopNotification()
//{}
void NotificationManager::PopNotification::render(GLCanvas3D& canvas, const float& initial_x)
{
	if (m_finnished)
		return;

	if (wxGetLocalTime() - m_creation_time >= m_data.duration)
		m_closed_by_user = true;

	if(m_closed_by_user)
	{
		m_finnished = true;
		canvas.request_extra_frame();
		return;
	}
	m_initial_x = initial_x;
	Size cnv_size = canvas.get_canvas_size();
	
	ImGuiWrapper& imgui = *wxGetApp().imgui();
	imgui.set_next_window_pos(1.0f * (float)cnv_size.get_width(), 1.0f * (float)cnv_size.get_height() - initial_x, ImGuiCond_Always, 1.0f, m_moving_anchor);

	if (m_moving_anchor < 1.0f)
		m_moving_anchor+= 0.1f;
	if (m_moving_anchor > 1.0f)
		m_moving_anchor = 1.0f;

	bool shown = true;
	std::string name = "";
	for (size_t i = 0; i < m_data.id; i++)name += " ";
	if (imgui.begin(name/*_(L("Notification"))*/, &shown, /*ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | */ImGuiWindowFlags_NoCollapse))
	{
		if (shown)
		{
			
			ImVec2 win_size = ImGui::GetWindowSize();
			m_window_height = win_size.y;
			m_window_width = win_size.x;
			/*
			if ((last_win_size.x != win_size.x) || (last_win_size.y != win_size.y))
			{
				last_win_size = win_size;
				canvas.request_extra_frame();
			}
			*/
			const ImVec4& color = ImGui::GetStyleColorVec4(ImGuiCol_Separator);
			ImGui::PushStyleColor(ImGuiCol_Text, color);
			//ImGui::Separator();
			imgui.text(m_data.text.c_str());
			ImGui::PopStyleColor();
			/*
			if (imgui.button(_(L("Close"))))
			{
				// the user clicked on the [Close] button
				m_closed_by_user = true;
				canvas.set_as_dirty();
			}
			*/
		}
		else
		{
			// the user clicked on the [X] button
			m_closed_by_user = true;
			canvas.set_as_dirty();
		}
	}

	imgui.end();
}

float NotificationManager::PopNotification::get_top() const 
{
	return m_moving_anchor * m_window_height + m_initial_x;
}

//------NotificationManager--------
NotificationManager::NotificationManager():
	m_next_id(0)
{}
NotificationManager::~NotificationManager()
{
	m_notification_container.clear();
}
void NotificationManager::push_notification(const std::string& text)
{
	NotificationData d = { text, m_next_id++, 5 };
	Notification n = { d };
	m_notification_container.push_back(n);
	std::cout << "PUSH: " << text << std::endl;
	//show_notifications();
}
void NotificationManager::show_notifications() const 
{
	print_to_console();
}
void NotificationManager::hide_notifications() const 
{}

void NotificationManager::render_notification(GLCanvas3D& canvas)
{
	
	for (auto& notification : m_notification_container)
	{
		if(!notification.poped)
		{
			notification.poped = true;
			m_pop_notifications.push_back(new PopNotification(notification.data));
		}
	}
	float last_x = 0.0f;
	for (auto it = m_pop_notifications.begin(); it != m_pop_notifications.end();)
	{
		if((*it)->get_finnished())
		{
			delete (*it);
			it = m_pop_notifications.erase(it);
		}else
		{
			(*it)->render(canvas, last_x);
			last_x = (*it)->get_top();
			++it;
		}
	}
	//for (auto& pop : m_pop_notifications){}

}

void NotificationManager::print_to_console() const 
{
	std::cout << "---Notifications---" << std::endl;
	for (size_t i = 0; i < m_notification_container.size(); i++){
		std::cout << "Notification " << i << ": " << m_notification_container[i].data.text << std::endl;
	}
}



}//namespace GUI
}//namespace Slic3r