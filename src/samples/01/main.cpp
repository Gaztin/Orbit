#include <math.h>
#include <time.h>

#include <orbit/core/events/window_event.h>
#include <orbit/core/application.h>
#include <orbit/core/asset.h>
#include <orbit/core/log.h>
#include <orbit/core/utility.h>
#include <orbit/core/window.h>
#include <orbit/graphics/constant_buffer.h>
#include <orbit/graphics/graphics_pipeline.h>
#include <orbit/graphics/index_buffer.h>
#include <orbit/graphics/render_context.h>
#include <orbit/graphics/shader.h>
#include <orbit/graphics/vertex_buffer.h>

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
	orb::shader m_vertexShader;
	orb::shader m_fragmentShader;
	orb::vertex_buffer m_triangleVertexBuffer;
	orb::index_buffer m_triangleIndexBuffer;
	orb::constant_buffer m_triangleConstantBuffer;
	orb::graphics_pipeline m_mainPipeline;
	float m_time;
};

struct vertex
{
	float x, y, z, w;
	float r, g, b, a;
};

const orb::vertex_layout vertexLayout =
{
	{"POSITION", orb::vertex_component::Vec4},
	{"COLOR", orb::vertex_component::Vec4},
};

const std::initializer_list<vertex> triangleVertices =
{
	{ -0.5f, -0.5f, 0.0f, 1.0f,   0.0f, 0.0f, 1.0f, 1.0f },
	{ -0.5f,  0.5f, 0.0f, 1.0f,   1.0f, 0.0f, 0.0f, 1.0f },
	{  0.5f, -0.5f, 0.0f, 1.0f,   0.0f, 0.0f, 0.0f, 1.0f },
	{  0.5f,  0.5f, 0.0f, 1.0f,   0.0f, 1.0f, 0.0f, 1.0f },
};

const std::initializer_list<uint16_t> triangleIndices =
{
	0, 1, 2,
	3, 2, 1,
};

std::tuple triangleConstants = std::make_tuple
(
	1.0f
);

sample_app::sample_app()
	: m_window(800, 600)
	, m_windowSubscription(m_window.subscribe(&sample_app::on_window_event))
	, m_renderContext(m_window, orb::graphics_api::DeviceDefault)
	, m_vertexShader(orb::shader_type::Vertex, orb::asset("shader.vs"))
	, m_fragmentShader(orb::shader_type::Fragment, orb::asset("shader.fs"))
	, m_triangleVertexBuffer(triangleVertices)
	, m_triangleIndexBuffer(triangleIndices)
	, m_triangleConstantBuffer(triangleConstants)
	, m_time(0.0f)
{
	m_window.set_title("Orbit sample #01");
	m_window.show();
	m_renderContext.set_clear_color(0.0f, 0.0f, 0.5f);

	m_mainPipeline.add_shader(m_vertexShader);
	m_mainPipeline.add_shader(m_fragmentShader);
	m_mainPipeline.describe_vertex_layout(vertexLayout);

	/* Load text asset and log its contents */
	{
		orb::asset testAsset("text.txt");
		const auto& txt = testAsset.get_data();
		orb::log_info(std::string(reinterpret_cast<const char*>(txt.data()), txt.size()));
	}
}

void sample_app::frame()
{
	m_time = static_cast<float>(clock()) / CLOCKS_PER_SEC;
	const float diffuse = 0.5f + (0.5f * sin(m_time * static_cast<float>(M_PI)));

	std::get<0>(triangleConstants) = diffuse;

	m_window.poll_events();
	m_renderContext.clear(orb::buffer_mask::Color | orb::buffer_mask::Depth);

	m_triangleVertexBuffer.bind();
	m_triangleIndexBuffer.bind();
	m_triangleConstantBuffer.bind(orb::shader_type::Vertex, 0);
	m_triangleConstantBuffer.update(triangleConstants);
	m_mainPipeline.draw(m_triangleIndexBuffer);

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
