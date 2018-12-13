#include <map>

#include <time.h>

#include <orbit/core/log.h>
#include <orbit/core/window.h>
#include <orbit/core/utility.h>
#include <orbit/graphics/render_context.h>
#include <orbit/graphics/vertex_buffer.h>

#if defined(ORB_OS_WINDOWS)
#include <windows.h>
#endif

class scoped_benchmark
{
public:
	scoped_benchmark(const std::string& verb)
		: m_verb(verb)
	{
		timespec_get(&m_startTimeSpec, TIME_UTC);
	}

	~scoped_benchmark()
	{
		timespec currTimeSpec;
		timespec_get(&currTimeSpec, TIME_UTC);
		const double dur = (currTimeSpec.tv_sec - m_startTimeSpec.tv_sec) + 0.000000001 * (currTimeSpec.tv_nsec - m_startTimeSpec.tv_nsec);
		orb::log_info(orb::format("[%.7fs] to %s.", dur, m_verb.c_str()));
	}

private:
	std::string m_verb;
	timespec m_startTimeSpec;
};

const std::map<orb::graphics_api, std::string> apiNames =
{
	{ orb::graphics_api::OpenGL, "OpenGL" },
	{ orb::graphics_api::D3D11, "Direct3D 11" },
};

static void prepare()
{
#if defined(ORB_OS_WINDOWS)
	HDC hdc = GetDC(nullptr);
	DescribePixelFormat(hdc, 0, 0, nullptr);
	ReleaseDC(nullptr, hdc);
#endif
}

void bench_window_create(size_t count)
{
	scoped_benchmark bench(orb::format("create %d windows", count));
	for (size_t i = 0; i < count; ++i)
	{
		orb::window w(1024, 1024);
	}
}

void bench_context_create(orb::graphics_api api, size_t count)
{
	orb::window w(1024, 1024);
	scoped_benchmark bench(orb::format("create %d %s contexts", count, apiNames.at(api).c_str()));
	for (size_t i = 0; i < count; ++i)
	{
		orb::render_context(w, api);
	}
}

void bench_context_clear(orb::graphics_api api, size_t count)
{
	orb::window w(1024, 1024);
	orb::render_context rc(w, api);
	//rc.make_current(w);
	rc.set_clear_color(1.0f, 0.0f, 1.0f);
	scoped_benchmark bench(orb::format("clear %s context %d times", apiNames.at(api).c_str(), count));
	for (size_t i = 0; i < count; ++i)
	{
		rc.clear(orb::buffer_mask::Color);
	}
}

void bench_vertex_buffer_create(orb::graphics_api api, size_t count, size_t bindCount)
{
	orb::window w(1024, 1024);
	orb::render_context rc(w, api);

	struct vertex
	{
		float x, y, z;
	};

	const std::initializer_list<vertex> vertices =
	{
		{ -0.5f,  0.5f, 0.0f },
		{  0.0f, -0.5f, 0.0f },
		{  0.5f,  0.5f, 0.0f },
	};

	scoped_benchmark bench(orb::format("create %d %s vertex buffers and bind them %d times each", count, apiNames.at(api).c_str(), bindCount));

	for (size_t i = 0; i < count; ++i)
	{
		orb::vertex_buffer vb(vertices);
		for (size_t j = 0; j < bindCount; ++j)
			vb.bind();
	}
}

int main(int /*argc*/, char* /*argv*/[])
{
	prepare();

	/* Window benchmarks. */
	orb::log_info("[__Window__]");
	bench_window_create(5);

	/* Graphics benchmarks. */
	for (orb::graphics_api api : {orb::graphics_api::OpenGL, orb::graphics_api::D3D11})
	{
		orb::log_info(orb::format("[__%s__]", apiNames.at(api).c_str()));
		bench_context_create(api, 10);
		bench_context_clear(api, 1000);
		bench_vertex_buffer_create(api, 100000, 1000);
	}

#if !defined(ORB_OS_ANDROID)
	getc(stdin);
#endif

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
