/*
 * Copyright (c) 2018 Sebastian Kylander http://gaztin.com/
 *
 * This software is provided 'as-is', without any express or implied warranty. In no event will
 * the authors be held liable for any damages arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose, including commercial
 * applications, and to alter it and redistribute it freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not claim that you wrote the
 *    original software. If you use this software in a product, an acknowledgment in the product
 *    documentation would be appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be misrepresented as
 *    being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 */

#include "entry_point.h"

#include <UIKit/UIKit.h>

#include "orbit/core/log.h"

@interface app_delegate : UIResponder<UIApplicationDelegate>
@property (strong, nonatomic) UIWindow* uiWindow;
@end

namespace orb
{
namespace platform
{

void entry_point(int argc, char* argv[])
{
	@autoreleasepool
	{
		UIApplicationMain(argc, argv, nil, NSStringFromClass([app_delegate class]));
	}
}

}
}

@implementation app_delegate

- (BOOL)application:(UIApplication*)application didFinishLaunchingWithOptions:(NSDictionary*)launchOptions
{
	orb::log_info("didFinishLaunchingWithOptions()");
	return YES;
}

- (void)applicationWillResignActive:(UIApplication*)application
{
	orb::log_info("applicationWillResignActive()");
}

- (void)applicationDidEnterBackground:(UIApplication*)application
{
	orb::log_info("applicationDidEnterBackground()");
}

- (void)applicationWillEnterForeground:(UIApplication*)application
{
	orb::log_info("applicationWillEnterForeground()");
}

- (void)applicationDidBecomeActive:(UIApplication*)application
{
	orb::log_info("applicationDidBecomeActive()");
}

- (void)applicationWillTerminate:(UIApplication*)application
{
	orb::log_info("applicationWillTerminate()");
}

@end
