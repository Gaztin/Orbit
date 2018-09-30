#include <orbit/core/events/window_event.h>
#include <orbit/core/log.h>
#include <orbit/core/stringf.h>
#include <orbit/core/window.h>
#include <orbit/graphics/render_context.h>

int main(int /*argc*/, char* /*argv*/[])
{
	orb::log_info("Started!");

	orb::window w(800, 600);
	auto windowSubscription = w.subscribe([](const orb::window_event& e)
	{
		switch (e.type)
		{
			case orb::window_event::Resize:
				orb::log_info(orb::stringf("Resized: (%d, %d)", e.data.resize.w, e.data.resize.h));
				break;

			case orb::window_event::Move:
				orb::log_info(orb::stringf("Moved: (%d, %d)", e.data.move.x, e.data.move.y));
				break;

			case orb::window_event::Defocus:
				orb::log_info("Defocus");
				break;

			case orb::window_event::Focus:
				orb::log_info("Focus");
				break;

			case orb::window_event::Suspend:
				orb::log_info("Suspend");
				break;

			case orb::window_event::Restore:
				orb::log_info("Restore");
				break;

			case orb::window_event::Close:
				orb::log_info("Close");
				break;

			default:
				break;
		}
	});
	w.set_title("Orbit sample #01");
	w.show();

	orb::render_context rc(w, orb::graphics_api::DeviceDefault);
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
