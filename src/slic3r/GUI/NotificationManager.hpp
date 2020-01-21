#ifndef slic3r_GUI_NotificationManager_hpp_
#define slic3r_GUI_notificationManager_hpp_

namespace Slic3r {
namespace GUI {

class notificationManager
{
public:
	notificationManager();
	~notificationManager();
private:
	// stack-like structure for keeping notifications - we want to keep them, if they among x(~5) top then show them, if confirmed by user then delete them. When they arrive show them in notification pop-up(if clicked by user then no to keep them?).
}

}//namespace GUI
}//namespace Slic3r

#endif //slic3r_GUI_NotificationManager_hpp_