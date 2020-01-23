#ifndef slic3r_GUI_NotificationManager_hpp_
#define slic3r_GUI_notificationManager_hpp_

#include <string>
#include <vector>

namespace Slic3r {
namespace GUI {

class GLCanvas3D;

class NotificationManager
{
public:

	struct NotificationData {
		const std::string text;
		const int id;
		const int duration;
	};
	struct Notification {
		const NotificationData data;
		bool poped = false;
	};

	//Pop notification - shows only once to user.
	class PopNotification
	{
	public:
		PopNotification(const NotificationData& n);
		//~PopNotificiation();
		void render(GLCanvas3D& canvas, const float& initial_x);
		bool get_finnished() const { return m_finnished; }
		float get_top() const;
	private:
		const NotificationData m_data;
		bool m_finnished;
		bool m_closed_by_user;
		float m_moving_anchor;
		float m_window_height;
		float m_window_width;
		float m_initial_x;
		long m_creation_time;
	};


	NotificationManager();
	~NotificationManager();

	void push_notification(const std::string& text);
	void show_notifications() const;
	void hide_notifications() const;

	void render_notification(GLCanvas3D& canvas);

	
private:

	void print_to_console() const;

	std::vector<Notification> m_notification_container;
	std::deque<PopNotification*> m_pop_notifications;
	int m_next_id;
};

}//namespace GUI
}//namespace Slic3r

#endif //slic3r_GUI_NotificationManager_hpp_