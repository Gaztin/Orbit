#include <orbit/core/log.h>
#include <orbit/core/window.h>

int main(int /*argc*/, char* /*argv*/[])
{
	orb::log_info("Started!\n");

	orb::window w(800, 600);
	w.show();
	while (w)
	{
		w.poll_events();
	}

	orb::log_info("Exited!\n");
	return 0;
}

#if defined(ORB_OS_ANDROID)
#include <orbit/core/android_app.h>
void android_main(android_app* app)
{
	orb::android_only::app = app;
	main(0, {});
}

#endif
