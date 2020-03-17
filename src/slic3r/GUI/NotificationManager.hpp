#ifndef slic3r_GUI_NotificationManager_hpp_
#define slic3r_GUI_notificationManager_hpp_

#include <string>
#include <vector>

namespace Slic3r {
namespace GUI {


class GLCanvas3D;

enum class NotificationType
{
	Undefined,
	SlicingComplete
};
class NotificationManager
{
public:

	struct NotificationData {
		NotificationType    type;
		const std::string   text;
		const int           duration;
		const int           id;
	};

	//Pop notification - shows only once to user.
	class PopNotification
	{
	public:
		enum class RenderResult
		{
			Finished,
			ClosePending,
			Static,
			Moving
		};
		PopNotification(const NotificationData& n);
		//~PopNotificiation(){}
		RenderResult render(GLCanvas3D& canvas, const float& initial_x);
		// close will dissapear notification on next render
		void close() { m_close_pending = true; }
		// data from newer notification of same type
		void update();
		bool get_finished() const { return m_finished; }
		float get_top() const;
		NotificationType get_type() const { return m_data.type; }
		
	private:
		const NotificationData m_data;

		long  m_creation_time;
		bool  m_finished      { false };
		bool  m_close_pending { false };
		float m_window_height { 0.0f };
		float m_window_width  { 0.0f };
		float m_current_x     { 0.0f };
		float m_target_x      { 0.0f };
		float m_move_step     { 0.0f };
	};


	NotificationManager();
	~NotificationManager();

	//pushes notification into the queue of notifications that are rendered
	void push_notification(const NotificationType type, const std::string& text, GLCanvas3D& canvas);
	// only type means standard text
	void push_notification(const NotificationType type, GLCanvas3D& canvas);
	// only text means Undefined type
	void push_notification(const std::string& text, GLCanvas3D& canvas);
	// renders notifications in queue and deletes expired ones
	void render_notifications(GLCanvas3D& canvas);

private:
	//finds older notification of same type and moves it to the end of queue. returns true if found
	bool find_older(NotificationType type);
	void print_to_console() const;

	std::deque<PopNotification*> m_pop_notifications;
	int m_next_id{ 0 };
};

}//namespace GUI
}//namespace Slic3r

#endif //slic3r_GUI_NotificationManager_hpp_