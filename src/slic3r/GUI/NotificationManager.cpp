#include "NotificationManager.hpp"

#include "GUI_App.hpp"
#include "GLCanvas3D.hpp"

#include <wx/glcanvas.h>
#include <iostream>

namespace Slic3r {
namespace GUI {

//------PopNotification--------
NotificationManager::PopNotification::PopNotification(const std::string& text) :
	 m_text(text)
	,m_finnished(false)
	
{
}
//NotificationManager::PopNotification::~PopNotification()
//{}
void NotificationManager::PopNotification::render(GLCanvas3D& canvas)
{
	if (m_finnished)
		return;

	Size cnv_size = canvas.get_canvas_size();
	
	ImGuiWrapper& imgui = *wxGetApp().imgui();
	imgui.set_next_window_pos(0.5f * (float)cnv_size.get_width(), 0.5f * (float)cnv_size.get_height(), ImGuiCond_Always, 0.5f, 0.5f);

	static ImVec2 last_win_size(0.0f, 0.0f);
	bool shown = true;
	if (imgui.begin(_(L("Notification")), &shown, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse))
	{
		if (shown)
		{
		}
		else
		{
			
		}
	}

	imgui.end();
	


	//std::cout << "NOTIFICATION: " << m_text << std::endl;
	//m_finnished = true;
}

//------NotificationManager--------
NotificationManager::NotificationManager():
	m_pop_notification(nullptr)
{}
NotificationManager::~NotificationManager()
{
	m_notification_container.clear();
}
void NotificationManager::push_notification(const std::string& text)
{
	Notification n = { text };
	m_notification_container.push_back(n);
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
	for (size_t i = 0; i < m_notification_container.size(); i++){
		if (!m_notification_container[i].poped) {
			if (m_pop_notification == nullptr) {
				m_notification_container[i].poped = true;
				m_pop_notification = new PopNotification(m_notification_container[i].text);
			}else if (m_pop_notification->get_finnished()) {
				delete m_pop_notification;
				m_pop_notification = nullptr;
			}
			if (m_pop_notification != nullptr)
				m_pop_notification->render(canvas);
			break;
		}
	}
}

void NotificationManager::print_to_console() const 
{
	std::cout << "---Notifications---" << std::endl;
	for (size_t i = 0; i < m_notification_container.size(); i++){
		std::cout << "Notification " << i << ": " << m_notification_container[i].text << std::endl;
	}
}



}//namespace GUI
}//namespace Slic3r