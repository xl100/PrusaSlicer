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
	// plan: stack-like structure for keeping notifications - we want to keep them, 
	// if they are among top x(~5) then show them (probably in some openable panel/window), if confirmed by user then delete them.
	// When they arrive show them in notification pop-up(if clicked by user then no to keep them?).
	// notifications should be pushable for any component - should this be (partialy) static?
	// should notifications "lead back"? after clicking show whats up. -needed only if doing "what gone wrong" notif
	// example notifications: sliced, sd card removed
}

}//namespace GUI
}//namespace Slic3r

#endif //slic3r_GUI_NotificationManager_hpp_