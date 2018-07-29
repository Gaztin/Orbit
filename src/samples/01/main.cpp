#include <orbit/core/log.h>
#include <orbit/core/window.h>
#include <orbit/graphics/render_context.h>

int main(int /*argc*/, char* /*argv*/[])
{
	orb::log_info("Started!");

	orb::window w(800, 600);
	orb::render_context rc(w, orb::graphics_api::DeviceDefault);
	w.set_title("Orbit sample #01");
	w.show();
	rc.make_current(w);
	rc.set_clear_color(1.0f, 0.0f, 1.0f);
	while (w)
	{
		w.poll_events();
		rc.clear(orb::buffer_mask::Color | orb::buffer_mask::Depth);

		rc.swap_buffers(w);
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
