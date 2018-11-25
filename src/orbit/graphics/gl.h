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

#pragma once
#include "orbit/graphics.h"

#if defined(ORB_HAS_OPENGL)

#include <string_view>
#include <stddef.h>

#if defined(ORB_OS_WINDOWS)
#include <windows.h>
#include <gl/GL.h>
#elif defined(ORB_OS_LINUX)
#include <GL/gl.h>
#elif defined(ORB_OS_MACOS)
#include <OpenGL/gl.h>
#elif defined(ORB_OS_ANDROID)
#include <GLES/gl.h>
#elif defined(ORB_OS_IOS)
#include <OpenGLES/ES1/gl.h>
#endif

namespace orb
{
namespace gl
{

using GLintptr = ptrdiff_t;
using GLsizeiptr = size_t;
using GLdouble = double;

enum class buffer_target : GLenum
{
	Array             = 0x8892,
	AtomicCounter     = 0x92c0,
	CopyRead          = 0x8f36,
	CopyWrite         = 0x8f37,
	DispatchIndirect  = 0x90ee,
	DrawIndirect      = 0x8f3f,
	ElementArray      = 0x8893,
	PixelPack         = 0x88eb,
	PixelUnpack       = 0x88ec,
	Query             = 0x9192,
	ShaderStorage     = 0x90d2,
	Texture           = 0x8c2a,
	TransformFeedback = 0x8c8e,
	Uniform           = 0x8a11,
};

enum class buffer_usage : GLenum
{
	StreamDraw  = 0x88e0,
	StreamRead  = 0x88e1,
	StreamCopy  = 0x88e2,
	StaticDraw  = 0x88e4,
	StaticRead  = 0x88e5,
	StaticCopy  = 0x88e6,
	DynamicDraw = 0x88e8,
	DynamicRead = 0x88e9,
	DynamicCopy = 0x88ea,
};

enum class draw_mode : GLenum
{
	Points                 = 0x0,
	LineStrip              = 0x3,
	LineLoop               = 0x2,
	Lines                  = 0x1,
	LineStripAdjacency     = 0xb,
	LinesAdjacency         = 0xa,
	TriangleStrip          = 0x5,
	TriangleFan            = 0x6,
	Triangles              = 0x4,
	TriangleStripAdjacency = 0xd,
	TriangleSAdjacency     = 0xc,
	Patches                = 0xe,
};

enum class index_type : GLenum
{
	Byte  = 0x1401,
	Short = 0x1403,
	Int   = 0x1405,
};

enum class buffer_param : GLenum
{
	Access = 0x88bb,
	Mapped = 0x88bc,
	Size   = 0x8764,
	Usage  = 0x8765,
};

enum class buffer_pointer_param : GLenum
{
	MapPointer = 0x88Bd,
};

enum class vertex_attrib_array_param : GLenum
{
	BufferBinding = 0x889f,
	Enabled       = 0x8622,
	Size          = 0x8623,
	Stride        = 0x8624,
	Type          = 0x8625,
	Normalized    = 0x886a,
	CurrentAttrib = 0x8626,
};

enum class vertex_attrib_array_pointer_param : GLenum
{
	Pointer = 0x8645,
};

enum class vertex_attrib_data_type : GLenum
{
	Byte          = 0x1400,
	UnsignedByte  = 0x1401,
	Short         = 0x1402,
	UnsignedShort = 0x1403,
	Fixed         = 0x140c,
	Float         = 0x1406,
};

extern ORB_API_GRAPHICS void (*bind_buffer)(buffer_target target, GLuint buffer);
extern ORB_API_GRAPHICS void (*buffer_data)(buffer_target target, GLsizeiptr size, const GLvoid* data, buffer_usage usage);
extern ORB_API_GRAPHICS void (*buffer_sub_data)(buffer_target target, GLintptr offset, GLsizeiptr size, const GLvoid* data);
extern ORB_API_GRAPHICS void (*delete_buffers)(GLsizei n, const GLuint* buffers);
extern ORB_API_GRAPHICS void (*disable_vertex_attrib_array)(GLuint index);
extern ORB_API_GRAPHICS void (*draw_arrays)(draw_mode mode, GLint first, GLsizei count);
extern ORB_API_GRAPHICS void (*draw_elements)(draw_mode, GLsizei count, index_type type, const GLvoid* indices);
extern ORB_API_GRAPHICS void (*enable_vertex_attrib_array)(GLuint index);
extern ORB_API_GRAPHICS void (*gen_buffers)(GLsizei n, GLuint* buffers);
extern ORB_API_GRAPHICS void (*get_buffer_parameteriv)(buffer_target target, buffer_param value, GLint* data);
extern ORB_API_GRAPHICS void (*get_buffer_pointerv)(buffer_target target, buffer_pointer_param pname, GLvoid** params);
extern ORB_API_GRAPHICS void (*get_vertex_attribdv)(GLuint index, vertex_attrib_array_param pname, GLdouble* params);
extern ORB_API_GRAPHICS void (*get_vertex_attribfv)(GLuint index, vertex_attrib_array_param pname, GLfloat* params);
extern ORB_API_GRAPHICS void (*get_vertex_attribiv)(GLuint index, vertex_attrib_array_param pname, GLint* params);
extern ORB_API_GRAPHICS void (*get_vertex_attrib_pointerv)(GLuint index, vertex_attrib_array_pointer_param pname, GLvoid** pointer);
extern ORB_API_GRAPHICS GLboolean (*is_buffer)(GLuint buffer);
extern ORB_API_GRAPHICS void (*vertex_attrib1f)(GLuint index, GLfloat v0);
extern ORB_API_GRAPHICS void (*vertex_attrib2f)(GLuint index, GLfloat v0, GLfloat v1);
extern ORB_API_GRAPHICS void (*vertex_attrib3f)(GLuint index, GLfloat v0, GLfloat v1, GLfloat v2);
extern ORB_API_GRAPHICS void (*vertex_attrib4f)(GLuint index, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
extern ORB_API_GRAPHICS void (*vertex_attrib_pointer)(GLuint index, GLint size, vertex_attrib_data_type type, GLboolean normalized, GLsizei stride, const GLvoid* pointer);

namespace platform
{

extern ORB_API_GRAPHICS void* get_proc_address(std::string_view name);

}

}
}

#endif
