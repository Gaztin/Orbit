/*
 * Copyright (c) 2019 Sebastian Kylander https://gaztin.com/
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
#include "Orbit/Graphics/API/OpenGL/OpenGLEnums.h"
#include "Orbit/Graphics/API/OpenGL/OpenGL.h"

ORB_NAMESPACE_BEGIN

#if defined( ORB_OS_WINDOWS )
#  define ORB_GL_CALL __stdcall
#else
#  define ORB_GL_CALL
#endif

template< char... Chars >
using OpenGLProcLiteral = std::integer_sequence< char, Chars... >;

template< typename >
struct OpenGLProcLiteralTraits;

template< char... Chars >
struct OpenGLProcLiteralTraits< OpenGLProcLiteral< Chars... > >
{
	static constexpr char proc_name[ sizeof...( Chars ) + 1 ] = { Chars..., '\0' };
};

template< typename PL, typename Func >
class OpenGLFunction;

template< typename PL, typename R, typename... Args >
class OpenGLFunction< PL, R( Args... ) >
{
public:

	OpenGLFunction( void )
		: m_ptr( nullptr )
	{
	}

	ORB_DISABLE_COPY_AND_MOVE( OpenGLFunction );

public:

	R operator()( Args... args )
	{
		using Proc = R( ORB_GL_CALL* )( Args... );

		if( m_ptr == nullptr )
		{
			m_ptr = GetOpenGLProcAddress( OpenGLProcLiteralTraits< PL >::proc_name );
		}

		// Reset error code to 0
		while( glGetError() != GL_NO_ERROR );

		if constexpr( std::is_void_v< R > )
		{
			reinterpret_cast< Proc >( m_ptr )( args... );
			HandleOpenGLError( glGetError(), OpenGLProcLiteralTraits< PL >::proc_name );
		}
		else
		{
			R res = reinterpret_cast< Proc >( m_ptr )( args... );
			HandleOpenGLError( glGetError(), OpenGLProcLiteralTraits< PL >::proc_name );
			return res;
		}
	}

private:

	void* m_ptr;

};

inline void glGetBooleanv( OpenGLStateParam pname, GLboolean* params ) { return ::glGetBooleanv( static_cast< GLenum >( pname ), params ); }
inline void glGetFloatv  ( OpenGLStateParam pname, GLfloat* params )   { return ::glGetFloatv(   static_cast< GLenum >( pname ), params ); }
inline void glGetIntegerv( OpenGLStateParam pname, GLint* params )     { return ::glGetIntegerv( static_cast< GLenum >( pname ), params ); }

inline OpenGLFunction< OpenGLProcLiteral< 'g','l','A','c','t','i','v','e','T','e','x','t','u','r','e' >,                                             void( OpenGLTextureUnit texture ) >                                                                                                                     glActiveTexture;
inline OpenGLFunction< OpenGLProcLiteral< 'g','l','A','t','t','a','c','h','S','h','a','d','e','r' >,                                                 void( GLuint program, GLuint shader )>                                                                                                                  glAttachShader;
inline OpenGLFunction< OpenGLProcLiteral< 'g','l','B','i','n','d','A','t','t','r','i','b','L','o','c','a','t','i','o','n' >,                         void( GLuint program, GLuint index, const GLchar* name )>                                                                                               glBindAttribLocation;
inline OpenGLFunction< OpenGLProcLiteral< 'g','l','B','i','n','d','B','u','f','f','e','r' >,                                                         void( OpenGLBufferTarget target, GLuint buffer )>                                                                                                       glBindBuffer;
inline OpenGLFunction< OpenGLProcLiteral< 'g','l','B','i','n','d','B','u','f','f','e','r','B','a','s','e' >,                                         void( OpenGLBufferTarget target, GLuint index, GLuint buffer )>                                                                                         glBindBufferBase;
inline OpenGLFunction< OpenGLProcLiteral< 'g','l','B','i','n','d','B','u','f','f','e','r','R','a','n','g','e' >,                                     void( OpenGLBufferTarget target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size )>                                                       glBindBufferRange;
inline OpenGLFunction< OpenGLProcLiteral< 'g','l','B','i','n','d','T','e','x','t','u','r','e' >,                                                     void( GLenum target, GLuint texture ) >                                                                                                                 glBindTexture;
inline OpenGLFunction< OpenGLProcLiteral< 'g','l','B','i','n','d','V','e','r','t','e','x','A','r','r','a','y' >,                                     void( GLuint array )>                                                                                                                                   glBindVertexArray;
inline OpenGLFunction< OpenGLProcLiteral< 'g','l','B','i','n','d','V','e','r','t','e','x','B','u','f','f','e','r' >,                                 void( GLuint bindingindex, GLuint buffer, GLintptr offset, GLintptr stride )>                                                                           glBindVertexBuffer;
inline OpenGLFunction< OpenGLProcLiteral< 'g','l','B','u','f','f','e','r','D','a','t','a' >,                                                         void( OpenGLBufferTarget target, GLsizeiptr size, const GLvoid* data, OpenGLBufferUsage usage )>                                                        glBufferData;
inline OpenGLFunction< OpenGLProcLiteral< 'g','l','B','u','f','f','e','r','S','u','b','D','a','t','a' >,                                             void( OpenGLBufferTarget target, GLintptr offset, GLsizeiptr size, const GLvoid* data )>                                                                glBufferSubData;
inline OpenGLFunction< OpenGLProcLiteral< 'g','l','C','o','m','p','i','l','e','S','h','a','d','e','r' >,                                             void( GLuint shader )>                                                                                                                                  glCompileShader;
inline OpenGLFunction< OpenGLProcLiteral< 'g','l','C','o','m','p','r','e','s','s','e','d','T','e','x','I','m','a','g','e','2','D' >,                 void( GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid* data ) >         glCompressedTexImage2D;
inline OpenGLFunction< OpenGLProcLiteral< 'g','l','C','o','m','p','r','e','s','s','e','d','T','e','x','S','u','b','I','m','a','g','e','2','D' >,     void( GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid* data ) > glCompressedTexSubImage2D;
inline OpenGLFunction< OpenGLProcLiteral< 'g','l','C','o','p','y','B','u','f','f','e','r','S','u','b','D','a','t','a' >,                             void( OpenGLBufferTarget readtarget, OpenGLBufferTarget writetarget, GLintptr readoffset, GLintptr writeoffset, GLsizeiptr size )>                      glCopyBufferSubData;
inline OpenGLFunction< OpenGLProcLiteral< 'g','l','C','o','p','y','T','e','x','I','m','a','g','e','2','D' >,                                         void( GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border ) >                              glCopyTexImage2D;
inline OpenGLFunction< OpenGLProcLiteral< 'g','l','C','o','p','y','T','e','x','S','u','b','I','m','a','g','e','2','D' >,                             void( GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height ) >                                     glCopyTexSubImage2D;
inline OpenGLFunction< OpenGLProcLiteral< 'g','l','C','r','e','a','t','e','P','r','o','g','r','a','m' >,                                             GLuint()>                                                                                                                                               glCreateProgram;
inline OpenGLFunction< OpenGLProcLiteral< 'g','l','C','r','e','a','t','e','S','h','a','d','e','r' >,                                                 GLuint( OpenGLShaderType type )>                                                                                                                        glCreateShader;
inline OpenGLFunction< OpenGLProcLiteral< 'g','l','D','e','l','e','t','e','B','u','f','f','e','r','s' >,                                             void( GLsizei n, const GLuint* buffers )>                                                                                                               glDeleteBuffers;
inline OpenGLFunction< OpenGLProcLiteral< 'g','l','D','e','l','e','t','e','P','r','o','g','r','a','m' >,                                             void( GLuint program )>                                                                                                                                 glDeleteProgram;
inline OpenGLFunction< OpenGLProcLiteral< 'g','l','D','e','l','e','t','e','S','h','a','d','e','r' >,                                                 void( GLuint shader )>                                                                                                                                  glDeleteShader;
inline OpenGLFunction< OpenGLProcLiteral< 'g','l','D','e','l','e','t','e','T','e','x','t','u','r','e','s' >,                                         void( GLsizei n, const GLuint* textures ) >                                                                                                             glDeleteTextures;
inline OpenGLFunction< OpenGLProcLiteral< 'g','l','D','e','l','e','t','e','V','e','r','t','e','x','A','r','r','a','y','s' >,                         void( GLsizei n, const GLuint* arrays )>                                                                                                                glDeleteVertexArrays;
inline OpenGLFunction< OpenGLProcLiteral< 'g','l','D','e','t','a','c','h','S','h','a','d','e','r' >,                                                 void( GLuint program, GLuint shader )>                                                                                                                  glDetachShader;
inline OpenGLFunction< OpenGLProcLiteral< 'g','l','D','i','s','a','b','l','e','V','e','r','t','e','x','A','t','t','r','i','b','A','r','r','a','y' >, void( GLuint index )>                                                                                                                                   glDisableVertexAttribArray;
inline OpenGLFunction< OpenGLProcLiteral< 'g','l','D','r','a','w','A','r','r','a','y','s' >,                                                         void( OpenGLDrawMode mode, GLint first, GLsizei count )>                                                                                                glDrawArrays;
inline OpenGLFunction< OpenGLProcLiteral< 'g','l','D','r','a','w','A','r','r','a','y','s','I','n','d','i','r','e','c','t' >,                         void( OpenGLDrawMode mode, const void* indirect )>                                                                                                      glDrawArraysIndirect;
inline OpenGLFunction< OpenGLProcLiteral< 'g','l','D','r','a','w','A','r','r','a','y','s','I','n','s','t','a','n','c','e','d' >,                     void( OpenGLDrawMode mode, GLint first, GLsizei count, GLsizei primcount )>                                                                             glDrawArraysInstanced;
inline OpenGLFunction< OpenGLProcLiteral< 'g','l','D','r','a','w','E','l','e','m','e','n','t','s' >,                                                 void( OpenGLDrawMode mode, GLsizei count, OpenGLIndexType type, const GLvoid* indices )>                                                                glDrawElements;
inline OpenGLFunction< OpenGLProcLiteral< 'g','l','D','r','a','w','E','l','e','m','e','n','t','s','I','n','d','i','r','e','c','t' >,                 void( OpenGLDrawMode mode, OpenGLIndexType type, const void* indirect )>                                                                                glDrawElementsIndirect;
inline OpenGLFunction< OpenGLProcLiteral< 'g','l','D','r','a','w','E','l','e','m','e','n','t','s','I','n','s','t','a','n','c','e','d' >,             void( OpenGLDrawMode mode, GLsizei count, OpenGLIndexType type, const void* indices, GLsizei primcount )>                                               glDrawElementsInstanced;
inline OpenGLFunction< OpenGLProcLiteral< 'g','l','D','r','a','w','R','a','n','g','e','E','l','e','m','e','n','t','s' >,                             void( OpenGLDrawMode mode, GLuint start, GLuint end, GLsizei count, OpenGLIndexType type, const GLvoid* indices )>                                      glDrawRangeElements;
inline OpenGLFunction< OpenGLProcLiteral< 'g','l','E','n','a','b','l','e','V','e','r','t','e','x','A','t','t','r','i','b','A','r','r','a','y' >,     void( GLuint index )>                                                                                                                                   glEnableVertexAttribArray;
inline OpenGLFunction< OpenGLProcLiteral< 'g','l','F','l','u','s','h','M','a','p','p','e','d','B','u','f','f','e','r','R','a','n','g','e' >,         GLsync( OpenGLBufferTarget target, GLintptr offset, GLsizeiptr length )>                                                                                glFlushMappedBufferRange;
inline OpenGLFunction< OpenGLProcLiteral< 'g','l','G','e','n','B','u','f','f','e','r','s' >,                                                         void( GLsizei n, GLuint* buffers )>                                                                                                                     glGenBuffers;
inline OpenGLFunction< OpenGLProcLiteral< 'g','l','G','e','n','T','e','x','t','u','r','e','s' >,                                                     void( GLsizei n, GLuint* textures ) >                                                                                                                   glGenTextures;
inline OpenGLFunction< OpenGLProcLiteral< 'g','l','G','e','n','V','e','r','t','e','x','A','r','r','a','y','s' >,                                     void( GLsizei n, GLuint* arrays )>                                                                                                                      glGenVertexArrays;
inline OpenGLFunction< OpenGLProcLiteral< 'g','l','G','e','t','A','c','t','i','v','e','A','t','t','r','i','b' >,                                     void( GLuint program, GLuint index, GLsizei bufSize, GLsizei* length, GLint* size, OpenGLAttribDataType* type, GLchar* name )>                          glGetActiveAttrib;
inline OpenGLFunction< OpenGLProcLiteral< 'g','l','G','e','t','A','c','t','i','v','e','U','n','i','f','o','r','m' >,                                 void( GLuint program, GLuint index, GLsizei bufSize, GLsizei* length, GLint* size, OpenGLUniformDataType* type, GLchar* name )>                         glGetActiveUniform;
inline OpenGLFunction< OpenGLProcLiteral< 'g','l','G','e','t','A','t','t','a','c','h','e','d','S','h','a','d','e','r','s' >,                         void( GLuint program, GLsizei maxCount, GLsizei* count, GLuint* shaders )>                                                                              glGetAttachedShaders;
inline OpenGLFunction< OpenGLProcLiteral< 'g','l','G','e','t','A','t','t','r','i','b','L','o','c','a','t','i','o','n' >,                             GLint( GLuint program, const GLchar* name )>                                                                                                            glGetAttribLocation;
inline OpenGLFunction< OpenGLProcLiteral< 'g','l','G','e','t','B','u','f','f','e','r','P','a','r','a','m','e','t','e','r','i','6','4','v' >,         void( OpenGLBufferTarget target, OpenGLBufferParam value, GLint64* data )>                                                                              glGetBufferParameteri64v;
inline OpenGLFunction< OpenGLProcLiteral< 'g','l','G','e','t','B','u','f','f','e','r','P','a','r','a','m','e','t','e','r','i','v' >,                 void( OpenGLBufferTarget target, OpenGLBufferParam value, GLint* data )>                                                                                glGetBufferParameteriv;
inline OpenGLFunction< OpenGLProcLiteral< 'g','l','G','e','t','B','u','f','f','e','r','P','o','i','n','t','e','r','v' >,                             void( OpenGLBufferTarget target, OpenGLBufferPointerParam pname, GLvoid** params )>                                                                     glGetBufferPointerv;
inline OpenGLFunction< OpenGLProcLiteral< 'g','l','G','e','t','P','r','o','g','r','a','m','I','n','f','o','L','o','g' >,                             void( GLuint program, GLsizei maxLength, GLsizei* length, GLchar* infoLog )>                                                                            glGetProgramInfoLog;
inline OpenGLFunction< OpenGLProcLiteral< 'g','l','G','e','t','P','r','o','g','r','a','m','i','v' >,                                                 void( GLuint program, OpenGLProgramParam pname, GLint* params )>                                                                                        glGetProgramiv;
inline OpenGLFunction< OpenGLProcLiteral< 'g','l','G','e','t','S','h','a','d','e','r','I','n','f','o','L','o','g' >,                                 void( GLuint shader, GLsizei maxLength, GLsizei* length, GLchar* infoLog )>                                                                             glGetShaderInfoLog;
inline OpenGLFunction< OpenGLProcLiteral< 'g','l','G','e','t','S','h','a','d','e','r','i','v' >,                                                     void( GLuint shader, OpenGLShaderParam pname, GLint* params )>                                                                                          glGetShaderiv;
inline OpenGLFunction< OpenGLProcLiteral< 'g','l','G','e','t','S','h','a','d','e','r','S','o','u','r','c','e' >,                                     void( GLuint shader, GLsizei bufSize, GLsizei* length, GLchar* source )>                                                                                glGetShaderSource;
inline OpenGLFunction< OpenGLProcLiteral< 'g','l','G','e','t','U','n','i','f','o','r','m','f','v' >,                                                 void( GLuint program, GLint location, GLfloat* params )>                                                                                                glGetUniformfv;
inline OpenGLFunction< OpenGLProcLiteral< 'g','l','G','e','t','U','n','i','f','o','r','m','i','v' >,                                                 void( GLuint program, GLint location, GLint* params )>                                                                                                  glGetUniformiv;
inline OpenGLFunction< OpenGLProcLiteral< 'g','l','G','e','t','U','n','i','f','o','r','m','L','o','c','a','t','i','o','n' >,                         GLint( GLuint program, const GLchar* name )>                                                                                                            glGetUniformLocation;
inline OpenGLFunction< OpenGLProcLiteral< 'g','l','G','e','t','V','e','r','t','e','x','A','t','t','r','i','b','f','v' >,                             void( GLuint index, OpenGLVertexAttribArrayParam pname, GLfloat* params )>                                                                              glGetVertexAttribfv;
inline OpenGLFunction< OpenGLProcLiteral< 'g','l','G','e','t','V','e','r','t','e','x','A','t','t','r','i','b','P','o','i','n','t','e','r','v' >,     void( GLuint index, OpenGLVertexAttribArrayPointerParam pname, GLvoid** pointer )>                                                                      glGetVertexAttribPointerv;
inline OpenGLFunction< OpenGLProcLiteral< 'g','l','I','s','B','u','f','f','e','r' >,                                                                 GLboolean( GLuint buffer )>                                                                                                                             glIsBuffer;
inline OpenGLFunction< OpenGLProcLiteral< 'g','l','I','s','P','r','o','g','r','a','m' >,                                                             GLboolean( GLuint program )>                                                                                                                            glIsProgram;
inline OpenGLFunction< OpenGLProcLiteral< 'g','l','I','s','S','h','a','d','e','r' >,                                                                 GLboolean( GLuint shader )>                                                                                                                             glIsShader;
inline OpenGLFunction< OpenGLProcLiteral< 'g','l','I','s','T','e','x','t','u','r','e' >,                                                             GLboolean( GLuint texture ) >                                                                                                                           glIsTexture;
inline OpenGLFunction< OpenGLProcLiteral< 'g','l','I','s','V','e','r','t','e','x','A','r','r','a','y' >,                                             GLboolean( GLuint array )>                                                                                                                              glIsVertexArray;
inline OpenGLFunction< OpenGLProcLiteral< 'g','l','L','i','n','k','P','r','o','g','r','a','m' >,                                                     void( GLuint program )>                                                                                                                                 glLinkProgram;
inline OpenGLFunction< OpenGLProcLiteral< 'g','l','M','a','p','B','u','f','f','e','r','R','a','n','g','e' >,                                         void* ( OpenGLBufferTarget target, GLintptr offset, GLsizeiptr length, OpenGLMapAccess access )>                                                        glMapBufferRange;
inline OpenGLFunction< OpenGLProcLiteral< 'g','l','S','h','a','d','e','r','S','o','u','r','c','e' >,                                                 void( GLuint shader, GLsizei count, const GLchar* const* string, const GLint* length )>                                                                 glShaderSource;
inline OpenGLFunction< OpenGLProcLiteral< 'g','l','T','e','x','S','u','b','I','m','a','g','e','2','D' >,                                             void( GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid* data ) >       glTexSubImage2D;
inline OpenGLFunction< OpenGLProcLiteral< 'g','l','U','n','i','f','o','r','m','1','f' >,                                                             void( GLint location, GLfloat v0 )>                                                                                                                     glUniform1f;
inline OpenGLFunction< OpenGLProcLiteral< 'g','l','U','n','i','f','o','r','m','1','f','v' >,                                                         void( GLint location, GLsizei count, const GLfloat* value )>                                                                                            glUniform1fv;
inline OpenGLFunction< OpenGLProcLiteral< 'g','l','U','n','i','f','o','r','m','1','i' >,                                                             void( GLint location, GLint v0 )>                                                                                                                       glUniform1i;
inline OpenGLFunction< OpenGLProcLiteral< 'g','l','U','n','i','f','o','r','m','1','i','v' >,                                                         void( GLint location, GLsizei count, const GLint* value )>                                                                                              glUniform1iv;
inline OpenGLFunction< OpenGLProcLiteral< 'g','l','U','n','i','f','o','r','m','2','f' >,                                                             void( GLint location, GLfloat v0, GLfloat v1 )>                                                                                                         glUniform2f;
inline OpenGLFunction< OpenGLProcLiteral< 'g','l','U','n','i','f','o','r','m','2','f','v' >,                                                         void( GLint location, GLsizei count, const GLfloat* value )>                                                                                            glUniform2fv;
inline OpenGLFunction< OpenGLProcLiteral< 'g','l','U','n','i','f','o','r','m','2','i' >,                                                             void( GLint location, GLint v0, GLint v1 )>                                                                                                             glUniform2i;
inline OpenGLFunction< OpenGLProcLiteral< 'g','l','U','n','i','f','o','r','m','2','i','v' >,                                                         void( GLint location, GLsizei count, const GLint* value )>                                                                                              glUniform2iv;
inline OpenGLFunction< OpenGLProcLiteral< 'g','l','U','n','i','f','o','r','m','3','f' >,                                                             void( GLint location, GLfloat v0, GLfloat v1, GLfloat v2 )>                                                                                             glUniform3f;
inline OpenGLFunction< OpenGLProcLiteral< 'g','l','U','n','i','f','o','r','m','3','f','v' >,                                                         void( GLint location, GLsizei count, const GLfloat* value )>                                                                                            glUniform3fv;
inline OpenGLFunction< OpenGLProcLiteral< 'g','l','U','n','i','f','o','r','m','3','i' >,                                                             void( GLint location, GLint v0, GLint v1, GLint v2 )>                                                                                                   glUniform3i;
inline OpenGLFunction< OpenGLProcLiteral< 'g','l','U','n','i','f','o','r','m','3','i','v' >,                                                         void( GLint location, GLsizei count, const GLint* value )>                                                                                              glUniform3iv;
inline OpenGLFunction< OpenGLProcLiteral< 'g','l','U','n','i','f','o','r','m','4','f' >,                                                             void( GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3 )>                                                                                 glUniform4f;
inline OpenGLFunction< OpenGLProcLiteral< 'g','l','U','n','i','f','o','r','m','4','f','v' >,                                                         void( GLint location, GLsizei count, const GLfloat* value )>                                                                                            glUniform4fv;
inline OpenGLFunction< OpenGLProcLiteral< 'g','l','U','n','i','f','o','r','m','4','i' >,                                                             void( GLint location, GLint v0, GLint v1, GLint v2, GLint v3 )>                                                                                         glUniform4i;
inline OpenGLFunction< OpenGLProcLiteral< 'g','l','U','n','i','f','o','r','m','4','i','v' >,                                                         void( GLint location, GLsizei count, const GLint* value )>                                                                                              glUniform4iv;
inline OpenGLFunction< OpenGLProcLiteral< 'g','l','U','n','m','a','p','B','u','f','f','e','r' >,                                                     GLboolean( OpenGLBufferTarget target )>                                                                                                                 glUnmapBuffer;
inline OpenGLFunction< OpenGLProcLiteral< 'g','l','U','s','e','P','r','o','g','r','a','m' >,                                                         void( GLuint program )>                                                                                                                                 glUseProgram;
inline OpenGLFunction< OpenGLProcLiteral< 'g','l','V','a','l','i','d','a','t','e','P','r','o','g','r','a','m' >,                                     void( GLuint program )>                                                                                                                                 glValidateProgram;
inline OpenGLFunction< OpenGLProcLiteral< 'g','l','V','e','r','t','e','x','A','t','t','r','i','b','1','f' >,                                         void( GLuint index, GLfloat v0 )>                                                                                                                       glVertexAttrib1f;
inline OpenGLFunction< OpenGLProcLiteral< 'g','l','V','e','r','t','e','x','A','t','t','r','i','b','1','f','v' >,                                     void( GLuint index, const GLfloat* v )>                                                                                                                 glVertexAttrib1fv;
inline OpenGLFunction< OpenGLProcLiteral< 'g','l','V','e','r','t','e','x','A','t','t','r','i','b','2','f' >,                                         void( GLuint index, GLfloat v0, GLfloat v1 )>                                                                                                           glVertexAttrib2f;
inline OpenGLFunction< OpenGLProcLiteral< 'g','l','V','e','r','t','e','x','A','t','t','r','i','b','2','f','v' >,                                     void( GLuint index, const GLfloat* v )>                                                                                                                 glVertexAttrib2fv;
inline OpenGLFunction< OpenGLProcLiteral< 'g','l','V','e','r','t','e','x','A','t','t','r','i','b','3','f' >,                                         void( GLuint index, GLfloat v0, GLfloat v1, GLfloat v2 )>                                                                                               glVertexAttrib3f;
inline OpenGLFunction< OpenGLProcLiteral< 'g','l','V','e','r','t','e','x','A','t','t','r','i','b','3','f','v' >,                                     void( GLuint index, const GLfloat* v )>                                                                                                                 glVertexAttrib3fv;
inline OpenGLFunction< OpenGLProcLiteral< 'g','l','V','e','r','t','e','x','A','t','t','r','i','b','4','f' >,                                         void( GLuint index, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3 )>                                                                                   glVertexAttrib4f;
inline OpenGLFunction< OpenGLProcLiteral< 'g','l','V','e','r','t','e','x','A','t','t','r','i','b','4','f','v' >,                                     void( GLuint index, const GLfloat* v )>                                                                                                                 glVertexAttrib4fv;
inline OpenGLFunction< OpenGLProcLiteral< 'g','l','V','e','r','t','e','x','A','t','t','r','i','b','B','i','n','d','i','n','g' >,                     void( GLuint attribindex, GLuint bindingindex )>                                                                                                        glVertexAttribBinding;
inline OpenGLFunction< OpenGLProcLiteral< 'g','l','V','e','r','t','e','x','A','t','t','r','i','b','D','i','v','i','s','o','r' >,                     void( GLuint index, GLuint divisor )>                                                                                                                   glVertexAttribDivisor;
inline OpenGLFunction< OpenGLProcLiteral< 'g','l','V','e','r','t','e','x','A','t','t','r','i','b','F','o','r','m','a','t' >,                         void( GLuint attribindex, GLint size, OpenGLVertexAttribDataType type, GLboolean normalized, GLuint relativeoffset )>                                   glVertexAttribFormat;
inline OpenGLFunction< OpenGLProcLiteral< 'g','l','V','e','r','t','e','x','A','t','t','r','i','b','I','4','i' >,                                     void( GLuint index, GLint v0, GLint v1, GLint v2, GLint v3 )>                                                                                           glVertexAttribI4i;
inline OpenGLFunction< OpenGLProcLiteral< 'g','l','V','e','r','t','e','x','A','t','t','r','i','b','I','4','i','v' >,                                 void( GLuint index, const GLint* v )>                                                                                                                   glVertexAttribI4iv;
inline OpenGLFunction< OpenGLProcLiteral< 'g','l','V','e','r','t','e','x','A','t','t','r','i','b','I','4','u','i' >,                                 void( GLuint index, GLuint v0, GLuint v1, GLuint v2, GLuint v3 )>                                                                                       glVertexAttribI4ui;
inline OpenGLFunction< OpenGLProcLiteral< 'g','l','V','e','r','t','e','x','A','t','t','r','i','b','I','4','u','i','v' >,                             void( GLuint index, const GLuint* v )>                                                                                                                  glVertexAttribI4uiv;
inline OpenGLFunction< OpenGLProcLiteral< 'g','l','V','e','r','t','e','x','A','t','t','r','i','b','I','F','o','r','m','a','t' >,                     void( GLuint attribindex, GLint size, OpenGLVertexAttribDataType type, GLuint relativeoffset )>                                                         glVertexAttribIFormat;
inline OpenGLFunction< OpenGLProcLiteral< 'g','l','V','e','r','t','e','x','A','t','t','r','i','b','P','o','i','n','t','e','r' >,                     void( GLuint index, GLint size, OpenGLVertexAttribDataType type, GLboolean normalized, GLsizei stride, const GLvoid* pointer )>                         glVertexAttribPointer;
inline OpenGLFunction< OpenGLProcLiteral< 'g','l','V','e','r','t','e','x','B','i','n','d','i','n','g','D','i','v','i','s','o','r' >,                 void( GLuint bindingindex, GLuint divisor )>                                                                                                            glVertexBindingDivisor;

ORB_NAMESPACE_END
