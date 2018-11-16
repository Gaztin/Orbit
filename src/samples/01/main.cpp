#include <math.h>
#include <time.h>

#include <orbit/core/events/window_event.h>
#include <orbit/core/application.h>
#include <orbit/core/log.h>
#include <orbit/core/utility.h>
#include <orbit/core/window.h>
#include <orbit/graphics/render_context.h>

class sample_app : public orb::application
{
public:
	sample_app();

	void frame() final override;
	operator bool() const final override { return !!m_window; }

	static void on_window_event(const orb::window_event& e);

private:
	orb::window m_window;
	orb::window::subscription_ptr m_windowSubscription;
	orb::render_context m_renderContext;
	float m_time;
};

sample_app::sample_app()
	: m_window(800, 600)
	, m_windowSubscription(m_window.subscribe(&sample_app::on_window_event))
	, m_renderContext(m_window, orb::graphics_api::DeviceDefault)
{
	m_window.set_title("Orbit sample #01");
	m_window.show();
	m_renderContext.set_clear_color(1.0f, 0.0f, 1.0f);
}

void sample_app::frame()
{
	m_time = static_cast<float>(clock()) / CLOCKS_PER_SEC;
	const float red   = 0.5f * (1.0f + cos(m_time * M_PI));
	const float green = 0.0f;
	const float blue  = 0.5f * (1.0f + sin(m_time * M_PI));

	m_window.poll_events();
	m_renderContext.set_clear_color(red, green, blue);
	m_renderContext.clear(orb::buffer_mask::Color | orb::buffer_mask::Depth);
	m_renderContext.swap_buffers();
}

void sample_app::on_window_event(const orb::window_event& e)
{
	switch (e.type)
	{
		case orb::window_event::Resize:
			orb::log_info(orb::format("Resized: (%d, %d)", e.data.resize.w, e.data.resize.h));
			break;

		case orb::window_event::Move:
			orb::log_info(orb::format("Moved: (%d, %d)", e.data.move.x, e.data.move.y));
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
}

#if defined(ORB_OS_ANDROID)

void android_main(android_app* app)
{
	orb::application::main<sample_app>(app);
}

#else

int main(int argc, char* argv[])
{
	orb::application::main<sample_app>(std::make_pair(argc, argv));
}

#endif
