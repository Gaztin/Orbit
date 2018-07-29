#include <sstream>

#include <time.h>

#include <orbit/core/log.h>
#include <orbit/core/window.h>
#include <orbit/core/utility.h>
#include <orbit/graphics/render_context.h>

void recreate_contexts(orb::graphics_api api, size_t count)
{
	orb::window w(1024, 1024);
	w.show();
	clock_t c1 = clock();
	for (size_t i = 0; i < count; ++i)
	{
		const float fraction = (orb::cast<float>(i) / count);
		volatile orb::render_context rc(w, api);
	}
	clock_t c2 = clock();

	std::stringstream ss;
	ss << "Created and destroyed " << count << " ";
	switch (api)
	{
		case orb::graphics_api::OpenGL: ss << "OpenGL"; break;
		case orb::graphics_api::D3D11: ss << "Direct3D 11"; break;
		default: ss << "<Unknown API>";
	}
	ss << " contexts in " << 0.001f * orb::cast<float>(c2 - c1) << " seconds.\n";
	orb::log_info(ss.str());
}

int main(int /*argc*/, char* /*argv*/[])
{
	recreate_contexts(orb::graphics_api::OpenGL, 100);
	recreate_contexts(orb::graphics_api::D3D11, 100);

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
