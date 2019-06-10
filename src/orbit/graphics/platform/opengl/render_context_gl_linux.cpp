/*
* Copyright (c) 2018 Sebastian Kylander https://gaztin.com/
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

#include "render_context_gl.h"

#include "orbit/core/platform/window_handle.h"
#include "orbit/graphics/platform/opengl/gl_version.h"

namespace orb
{
	namespace platform
	{
		static int get_context_major( gl::version v )
		{
			switch( v )
			{
				case gl::version::v2_0: return 2;
				case gl::version::v3_2: return 3;
				case gl::version::v4_1: return 4;
				default:                return 0;
			}
		}

		static int get_context_minor( gl::version v )
		{
			switch( v )
			{
				case gl::version::v2_0: return 0;
				case gl::version::v3_2: return 2;
				case gl::version::v4_1: return 1;
				default:                return 0;
			}
		}

		static GC create_gc( Display* display, const Window& window )
		{
			return XCreateGC( display, window, 0, nullptr );
		}

		static GLXContext create_glx_context( Display* display, gl::version v )
		{
			int screen = DefaultScreen( display );
			int attribs[] =
			{
				GLX_X_RENDERABLE,  True,
				GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
				GLX_RENDER_TYPE,   GLX_RGBA_BIT,
				GLX_X_VISUAL_TYPE, GLX_TRUE_COLOR,
				GLX_RED_SIZE,      8,
				GLX_GREEN_SIZE,    8,
				GLX_BLUE_SIZE,     8,
				GLX_ALPHA_SIZE,    8,
				GLX_DEPTH_SIZE,    24,
				GLX_STENCIL_SIZE,  8,
				GLX_DOUBLEBUFFER,  True,
				None
			};

			do
			{
				int major, minor;
				if( !glXQueryVersion( display, &major, &minor ) )
					break;
				if( ( major < 1 ) || ( major == 1 && minor < 3 ) )
					break;

				using glXCreateContextAttribsARB_t = GLXContext( * )( Display * dpy, GLXFBConfig config, GLXContext shareContext, Bool direct, const int* attribList );
				glXCreateContextAttribsARB_t glXCreateContextAttribsARB = nullptr;
				glXCreateContextAttribsARB = reinterpret_cast< glXCreateContextAttribsARB_t >( glXGetProcAddressARB( reinterpret_cast< const GLubyte* >( "glXCreateContextAttribsARB" ) ) );
				if( !glXCreateContextAttribsARB )
					break;

				int fbConfigCount = 0;
				GLXFBConfig* fbConfigs = glXChooseFBConfig( display, screen, attribs, &fbConfigCount );
				if( !fbConfigs )
					break;
				if( fbConfigCount == 0 )
					break;

				// Choose the best config
				int bestFbConfigIdx = 0, bestSampleCount = 0;
				for( int i = 0; i < fbConfigCount; ++i )
				{
					XVisualInfo* vi = glXGetVisualFromFBConfig( display, fbConfigs[ i ] );
					if( vi )
					{
						int samples = 0, sampleCount = 0;
						glXGetFBConfigAttrib( display, fbConfigs[ i ], GLX_SAMPLE_BUFFERS, &samples );
						glXGetFBConfigAttrib( display, fbConfigs[ i ], GLX_SAMPLES, &sampleCount );

						if( samples && sampleCount > bestSampleCount )
						{
							bestFbConfigIdx = i;
							bestSampleCount = sampleCount;
						}
					}
					XFree( vi );
				}

				GLXFBConfig bestFbConfig = fbConfigs[ bestFbConfigIdx ];

				XFree( fbConfigs );

				int contextAttribs[] =
				{
					GLX_CONTEXT_MAJOR_VERSION_ARB, get_context_major( v ),
					GLX_CONTEXT_MINOR_VERSION_ARB, get_context_minor( v ),
					GLX_CONTEXT_FLAGS_ARB,         GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
					None
				};

				return glXCreateContextAttribsARB( display, bestFbConfig, 0, True, contextAttribs );

			} while( false );

			// If all else fails, use legacy method
			XVisualInfo* visualInfo = glXChooseVisual( display, screen, attribs );
			return glXCreateContext( display, visualInfo, nullptr, true );
		}

		static gl::functions init_functions( render_context_gl* self )
		{
			self->make_current();
			return gl::functions{};
		}

		render_context_gl::render_context_gl( const window_handle& wh, gl::version v )
			: m_wndPtr( &wh )
			, m_gc( create_gc( wh.display, wh.window ) )
			, m_glxContext( create_glx_context( wh.display, v ) )
			, m_functions( init_functions( this ) )
		{
			make_current( nullptr );
		}

		render_context_gl::~render_context_gl()
		{
			glXDestroyContext( m_wndPtr->display, m_glxContext );
			XFreeGC( m_wndPtr->display, m_gc );
		}

		bool render_context_gl::make_current()
		{
			return glXMakeCurrent( m_wndPtr->display, m_wndPtr->window, m_glxContext );
		}

		bool render_context_gl::make_current( std::nullptr_t )
		{
			return glXMakeCurrent( m_wndPtr->display, None, nullptr );
		}

		void render_context_gl::resize( uint32_t width, uint32_t height )
		{
			glViewport( 0, 0, width, height );
		}

		void render_context_gl::swap_buffers()
		{
			glXSwapBuffers( m_wndPtr->display, m_wndPtr->window );
		}

		void render_context_gl::set_clear_color( float r, float g, float b )
		{
			glClearColor( r, g, b, 1.0f );
		}

		void render_context_gl::clear_buffers( buffer_mask mask )
		{
			glClear( ( !!( mask & buffer_mask::Color ) ) ? GL_COLOR_BUFFER_BIT : 0 | ( !!( mask & buffer_mask::Depth ) ) ? GL_DEPTH_BUFFER_BIT : 0 );
		}
	}
}
