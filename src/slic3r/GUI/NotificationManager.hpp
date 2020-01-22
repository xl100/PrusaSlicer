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
	
	struct Notification {
		std::string text;
		bool poped = false;
		//Notification(const std::string& s) : text(s) {}
	};

	//Pop notification - shows only once to user.
	class PopNotification
	{
	public:
		PopNotification(const std::string& text);
		//~PopNotificiation();
		void render(GLCanvas3D& canvas);
		bool get_finnished() const { return m_finnished; }
	private:
		std::string  m_text;
		bool m_finnished;
	};


	NotificationManager();
	~NotificationManager();

	void push_notification(const std::string &text);
	void show_notifications() const;
	void hide_notifications() const;

	void render_notification(GLCanvas3D& canvas);

	
private:
	// plan: structure for keeping notifications - we want to keep them, 
	// if they are among top x(~5) then show them (probably in some openable panel/window), if confirmed by user then delete them.
	// When they arrive show them in notification pop-up(if clicked by user then no to keep them?).
	// notifications should be pushable for any component - should this be (partialy) static?
	// should notifications "lead back"? after clicking show whats up. -needed only if doing "what gone wrong" notif
	// example notifications: sliced, sd card removed

	void print_to_console() const;

	//container: operations needed:
	//	push_back (or front?)
	//	erase at any index
	//	access at any index 
	//vector/deque both can access&add O(1), deletes O(n). List access O(n), adds and deletes O(1)
	//lets do vector for now
	std::vector<Notification> m_notification_container;
	PopNotification* m_pop_notification;
};

}//namespace GUI
}//namespace Slic3r

#endif //slic3r_GUI_NotificationManager_hpp_