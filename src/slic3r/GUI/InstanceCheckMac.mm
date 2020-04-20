#import "InstanceCheck.hpp"
#import "InstanceCheckMac.h"
#import "GUI_App.hpp"

@implementation OtherInstanceMessageHandlerMac

-(instancetype) init
{
	self = [super init];
	return self;
}
-(void)add_observer
{
	NSLog(@"adding observer");
	[[NSDistributedNotificationCenter defaultCenter] addObserver:self selector:@selector(message_update:) name:@"OtherPrusaSlicerInstanceMessage" object:nil];
}

-(void)message_update:(NSNotification *)msg
{
	NSLog(@"recieved msg %@", msg);
	//NSLog(@"userinfo %@", msg.userInfo);
	//NSLog(@"userinfo data %@", msg.userInfo[@"data"]);
	//maximize window
	//[[[NSApplication sharedApplication] mainWindow] deminiaturize];
	//bring window to front 
	[[NSApplication sharedApplication] activateIgnoringOtherApps : YES];
	//pass message  
	Slic3r::GUI::wxGetApp().other_instance_message_handler()->handle_message(std::string([msg.userInfo[@"data"] UTF8String]));

	//NSLog(@"adding observer");
	//[[NSDistributedNotificationCenter defaultCenter] addObserver:self selector:@selector(message_update:) name:@"OtherPrusaSlicerInstanceMessage" object:nil];
}

@end

namespace Slic3r {

void send_message_mac(const std::string msg)
{
	NSString *nsmsg = [NSString stringWithCString:msg.c_str() encoding:[NSString defaultCStringEncoding]];
	NSLog(@"sending msg %@", nsmsg);
	[[NSDistributedNotificationCenter defaultCenter] postNotificationName:@"OtherPrusaSlicerInstanceMessage" object:nil userInfo:[NSDictionary dictionaryWithObject:nsmsg forKey:@"data"]];
}

namespace GUI {
void OtherInstanceMessageHandler::register_for_messages()
{
	m_impl_osx = [[OtherInstanceMessageHandlerMac alloc] init];
	if(m_impl_osx) {
		[m_impl_osx add_observer];
	}
}
void OtherInstanceMessageHandler::unregister_for_messages()
{
	if (m_impl_osx) {
        [m_impl_osx release];
        m_impl_osx = nullptr;
    } else {
		NSLog(@"unreegister not requred");
	}
}
}//namespace GUI
}//namespace Slicer


