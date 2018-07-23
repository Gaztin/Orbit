#include <orbit/core/log.h>

int main(int /*argc*/, char* /*argv*/[])
{
	orb::log_info("Started!\n");
	orb::log_info("Exited!\n");
	return 0;
}

#if defined(ORB_OS_ANDROID)
#include <android_native_app_glue.h>
void android_main(android_app* state)
{
	main(0, {});
}

#endif
