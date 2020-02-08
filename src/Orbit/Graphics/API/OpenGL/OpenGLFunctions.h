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
#include "Orbit/Core/Utility/StringLiteral.h"
#include "Orbit/Graphics/API/OpenGL/OpenGLEnums.h"
#include "Orbit/Graphics/API/OpenGL/OpenGL.h"

ORB_NAMESPACE_BEGIN

#if defined( ORB_OS_WINDOWS )
#  define ORB_GL_CALL __stdcall
#else
#  define ORB_GL_CALL
#endif

template< typename SL, typename Func >
class OpenGLFunction;

template< typename SL, typename R, typename... Args >
class OpenGLFunction< SL, R( Args... ) >
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
			m_ptr = GetOpenGLProcAddress( SL::value );
		}

		// Reset error code to 0
		while( glGetError() != GL_NO_ERROR );

		if constexpr( std::is_void_v< R > )
		{
			reinterpret_cast< Proc >( m_ptr )( args... );
			HandleOpenGLError( glGetError(), SL::value );
		}
		else
		{
			R res = reinterpret_cast< Proc >( m_ptr )( args... );
			HandleOpenGLError( glGetError(), SL::value );
			return res;
		}
	}

private:

	void* m_ptr;

};

inline void glGetBooleanv( OpenGLStateParam pname, GLboolean* params ) { return ::glGetBooleanv( static_cast< GLenum >( pname ), params ); }
inline void glGetFloatv  ( OpenGLStateParam pname, GLfloat* params )   { return ::glGetFloatv(   static_cast< GLenum >( pname ), params ); }
inline void glGetIntegerv( OpenGLStateParam pname, GLint* params )     { return ::glGetIntegerv( static_cast< GLenum >( pname ), params ); }

inline OpenGLFunction< ORB_STRING_LITERAL_32( "glActiveTexture" ),            void( OpenGLTextureUnit texture ) >                                                                                                                     glActiveTexture;
inline OpenGLFunction< ORB_STRING_LITERAL_32( "glAttachShader" ),             void( GLuint program, GLuint shader ) >                                                                                                                 glAttachShader;
inline OpenGLFunction< ORB_STRING_LITERAL_32( "glBindAttribLocation" ),       void( GLuint program, GLuint index, const GLchar* name ) >                                                                                              glBindAttribLocation;
inline OpenGLFunction< ORB_STRING_LITERAL_32( "glBindBuffer" ),               void( OpenGLBufferTarget target, GLuint buffer ) >                                                                                                      glBindBuffer;
inline OpenGLFunction< ORB_STRING_LITERAL_32( "glBindBufferBase" ),           void( OpenGLBufferTarget target, GLuint index, GLuint buffer ) >                                                                                        glBindBufferBase;
inline OpenGLFunction< ORB_STRING_LITERAL_32( "glBindBufferRange" ),          void( OpenGLBufferTarget target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size ) >                                                      glBindBufferRange;
inline OpenGLFunction< ORB_STRING_LITERAL_32( "glBindTexture" ),              void( GLenum target, GLuint texture ) >                                                                                                                 glBindTexture;
inline OpenGLFunction< ORB_STRING_LITERAL_32( "glBindVertexArray" ),          void( GLuint array ) >                                                                                                                                  glBindVertexArray;
inline OpenGLFunction< ORB_STRING_LITERAL_32( "glBindVertexBuffer" ),         void( GLuint bindingindex, GLuint buffer, GLintptr offset, GLintptr stride ) >                                                                          glBindVertexBuffer;
inline OpenGLFunction< ORB_STRING_LITERAL_32( "glBufferData" ),               void( OpenGLBufferTarget target, GLsizeiptr size, const GLvoid* data, OpenGLBufferUsage usage ) >                                                       glBufferData;
inline OpenGLFunction< ORB_STRING_LITERAL_32( "glBufferSubData" ),            void( OpenGLBufferTarget target, GLintptr offset, GLsizeiptr size, const GLvoid* data ) >                                                               glBufferSubData;
inline OpenGLFunction< ORB_STRING_LITERAL_32( "glCompileShader" ),            void( GLuint shader ) >                                                                                                                                 glCompileShader;
inline OpenGLFunction< ORB_STRING_LITERAL_32( "glCompressedTexImage2D" ),     void( GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid* data ) >         glCompressedTexImage2D;
inline OpenGLFunction< ORB_STRING_LITERAL_32( "glCompressedTexSubImage2D" ),  void( GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid* data ) > glCompressedTexSubImage2D;
inline OpenGLFunction< ORB_STRING_LITERAL_32( "glCopyBufferSubData" ),        void( OpenGLBufferTarget readtarget, OpenGLBufferTarget writetarget, GLintptr readoffset, GLintptr writeoffset, GLsizeiptr size ) >                     glCopyBufferSubData;
inline OpenGLFunction< ORB_STRING_LITERAL_32( "glCopyTexImage2D" ),           void( GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border ) >                              glCopyTexImage2D;
inline OpenGLFunction< ORB_STRING_LITERAL_32( "glCopyTexSubImage2D" ),        void( GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height ) >                                     glCopyTexSubImage2D;
inline OpenGLFunction< ORB_STRING_LITERAL_32( "glCreateProgram" ),            GLuint( void ) >                                                                                                                                        glCreateProgram;
inline OpenGLFunction< ORB_STRING_LITERAL_32( "glCreateShader" ),             GLuint( OpenGLShaderType type ) >                                                                                                                       glCreateShader;
inline OpenGLFunction< ORB_STRING_LITERAL_32( "glDeleteBuffers" ),            void( GLsizei n, const GLuint* buffers ) >                                                                                                              glDeleteBuffers;
inline OpenGLFunction< ORB_STRING_LITERAL_32( "glDeleteProgram" ),            void( GLuint program ) >                                                                                                                                glDeleteProgram;
inline OpenGLFunction< ORB_STRING_LITERAL_32( "glDeleteShader" ),             void( GLuint shader ) >                                                                                                                                 glDeleteShader;
inline OpenGLFunction< ORB_STRING_LITERAL_32( "glDeleteTextures" ),           void( GLsizei n, const GLuint* textures ) >                                                                                                             glDeleteTextures;
inline OpenGLFunction< ORB_STRING_LITERAL_32( "glDeleteVertexArrays" ),       void( GLsizei n, const GLuint* arrays ) >                                                                                                               glDeleteVertexArrays;
inline OpenGLFunction< ORB_STRING_LITERAL_32( "glDetachShader" ),             void( GLuint program, GLuint shader ) >                                                                                                                 glDetachShader;
inline OpenGLFunction< ORB_STRING_LITERAL_32( "glDisableVertexAttribArray" ), void( GLuint index ) >                                                                                                                                  glDisableVertexAttribArray;
inline OpenGLFunction< ORB_STRING_LITERAL_32( "glDrawArrays" ),               void( OpenGLDrawMode mode, GLint first, GLsizei count ) >                                                                                               glDrawArrays;
inline OpenGLFunction< ORB_STRING_LITERAL_32( "glDrawArraysIndirect" ),       void( OpenGLDrawMode mode, const void* indirect ) >                                                                                                     glDrawArraysIndirect;
inline OpenGLFunction< ORB_STRING_LITERAL_32( "glDrawArraysInstanced" ),      void( OpenGLDrawMode mode, GLint first, GLsizei count, GLsizei primcount ) >                                                                            glDrawArraysInstanced;
inline OpenGLFunction< ORB_STRING_LITERAL_32( "glDrawElements" ),             void( OpenGLDrawMode mode, GLsizei count, OpenGLIndexType type, const GLvoid* indices ) >                                                               glDrawElements;
inline OpenGLFunction< ORB_STRING_LITERAL_32( "glDrawElementsIndirect" ),     void( OpenGLDrawMode mode, OpenGLIndexType type, const void* indirect ) >                                                                               glDrawElementsIndirect;
inline OpenGLFunction< ORB_STRING_LITERAL_32( "glDrawElementsInstanced" ),    void( OpenGLDrawMode mode, GLsizei count, OpenGLIndexType type, const void* indices, GLsizei primcount ) >                                              glDrawElementsInstanced;
inline OpenGLFunction< ORB_STRING_LITERAL_32( "glDrawRangeElements" ),        void( OpenGLDrawMode mode, GLuint start, GLuint end, GLsizei count, OpenGLIndexType type, const GLvoid* indices ) >                                     glDrawRangeElements;
inline OpenGLFunction< ORB_STRING_LITERAL_32( "glEnableVertexAttribArray" ),  void( GLuint index ) >                                                                                                                                  glEnableVertexAttribArray;
inline OpenGLFunction< ORB_STRING_LITERAL_32( "glFlushMappedBufferRange" ),   GLsync( OpenGLBufferTarget target, GLintptr offset, GLsizeiptr length ) >                                                                               glFlushMappedBufferRange;
inline OpenGLFunction< ORB_STRING_LITERAL_32( "glGenBuffers" ),               void( GLsizei n, GLuint* buffers ) >                                                                                                                    glGenBuffers;
inline OpenGLFunction< ORB_STRING_LITERAL_32( "glGenTextures" ),              void( GLsizei n, GLuint* textures ) >                                                                                                                   glGenTextures;
inline OpenGLFunction< ORB_STRING_LITERAL_32( "glGenVertexArrays" ),          void( GLsizei n, GLuint* arrays ) >                                                                                                                     glGenVertexArrays;
inline OpenGLFunction< ORB_STRING_LITERAL_32( "glGetActiveAttrib" ),          void( GLuint program, GLuint index, GLsizei bufSize, GLsizei* length, GLint* size, OpenGLAttribDataType* type, GLchar* name ) >                         glGetActiveAttrib;
inline OpenGLFunction< ORB_STRING_LITERAL_32( "glGetActiveUniformBlockiv" ),  void( GLuint program, GLuint uniformBlockIndex, OpenGLUniformBlockParam pname, GLint* params ) >                                                        glGetActiveUniformBlockiv;
inline OpenGLFunction< ORB_STRING_LITERAL_32( "glGetActiveUniform" ),         void( GLuint program, GLuint index, GLsizei bufSize, GLsizei* length, GLint* size, OpenGLUniformDataType* type, GLchar* name ) >                        glGetActiveUniform;
inline OpenGLFunction< ORB_STRING_LITERAL_32( "glGetAttachedShaders" ),       void( GLuint program, GLsizei maxCount, GLsizei* count, GLuint* shaders ) >                                                                             glGetAttachedShaders;
inline OpenGLFunction< ORB_STRING_LITERAL_32( "glGetAttribLocation" ),        GLint( GLuint program, const GLchar* name ) >                                                                                                           glGetAttribLocation;
inline OpenGLFunction< ORB_STRING_LITERAL_32( "glGetBufferParameteri64v" ),   void( OpenGLBufferTarget target, OpenGLBufferParam value, GLint64* data ) >                                                                             glGetBufferParameteri64v;
inline OpenGLFunction< ORB_STRING_LITERAL_32( "glGetBufferParameteriv" ),     void( OpenGLBufferTarget target, OpenGLBufferParam value, GLint* data ) >                                                                               glGetBufferParameteriv;
inline OpenGLFunction< ORB_STRING_LITERAL_32( "glGetBufferPointerv" ),        void( OpenGLBufferTarget target, OpenGLBufferPointerParam pname, GLvoid** params ) >                                                                    glGetBufferPointerv;
inline OpenGLFunction< ORB_STRING_LITERAL_32( "glGetProgramInfoLog" ),        void( GLuint program, GLsizei maxLength, GLsizei* length, GLchar* infoLog ) >                                                                           glGetProgramInfoLog;
inline OpenGLFunction< ORB_STRING_LITERAL_32( "glGetProgramiv" ),             void( GLuint program, OpenGLProgramParam pname, GLint* params ) >                                                                                       glGetProgramiv;
inline OpenGLFunction< ORB_STRING_LITERAL_32( "glGetShaderInfoLog" ),         void( GLuint shader, GLsizei maxLength, GLsizei* length, GLchar* infoLog ) >                                                                            glGetShaderInfoLog;
inline OpenGLFunction< ORB_STRING_LITERAL_32( "glGetShaderiv" ),              void( GLuint shader, OpenGLShaderParam pname, GLint* params ) >                                                                                         glGetShaderiv;
inline OpenGLFunction< ORB_STRING_LITERAL_32( "glGetShaderSource" ),          void( GLuint shader, GLsizei bufSize, GLsizei* length, GLchar* source ) >                                                                               glGetShaderSource;
inline OpenGLFunction< ORB_STRING_LITERAL_32( "glGetUniformfv" ),             void( GLuint program, GLint location, GLfloat* params ) >                                                                                               glGetUniformfv;
inline OpenGLFunction< ORB_STRING_LITERAL_32( "glGetUniformiv" ),             void( GLuint program, GLint location, GLint* params ) >                                                                                                 glGetUniformiv;
inline OpenGLFunction< ORB_STRING_LITERAL_32( "glGetUniformLocation" ),       GLint( GLuint program, const GLchar* name ) >                                                                                                           glGetUniformLocation;
inline OpenGLFunction< ORB_STRING_LITERAL_32( "glGetVertexAttribfv" ),        void( GLuint index, OpenGLVertexAttribArrayParam pname, GLfloat* params ) >                                                                             glGetVertexAttribfv;
inline OpenGLFunction< ORB_STRING_LITERAL_32( "glGetVertexAttribPointerv" ),  void( GLuint index, OpenGLVertexAttribArrayPointerParam pname, GLvoid** pointer ) >                                                                     glGetVertexAttribPointerv;
inline OpenGLFunction< ORB_STRING_LITERAL_32( "glIsBuffer" ),                 GLboolean( GLuint buffer ) >                                                                                                                            glIsBuffer;
inline OpenGLFunction< ORB_STRING_LITERAL_32( "glIsProgram" ),                GLboolean( GLuint program ) >                                                                                                                           glIsProgram;
inline OpenGLFunction< ORB_STRING_LITERAL_32( "glIsShader" ),                 GLboolean( GLuint shader ) >                                                                                                                            glIsShader;
inline OpenGLFunction< ORB_STRING_LITERAL_32( "glIsTexture" ),                GLboolean( GLuint texture ) >                                                                                                                           glIsTexture;
inline OpenGLFunction< ORB_STRING_LITERAL_32( "glIsVertexArray" ),            GLboolean( GLuint array ) >                                                                                                                             glIsVertexArray;
inline OpenGLFunction< ORB_STRING_LITERAL_32( "glLinkProgram" ),              void( GLuint program ) >                                                                                                                                glLinkProgram;
inline OpenGLFunction< ORB_STRING_LITERAL_32( "glMapBufferRange" ),           void* ( OpenGLBufferTarget target, GLintptr offset, GLsizeiptr length, OpenGLMapAccess access ) >                                                       glMapBufferRange;
inline OpenGLFunction< ORB_STRING_LITERAL_32( "glShaderSource" ),             void( GLuint shader, GLsizei count, const GLchar* const* string, const GLint* length ) >                                                                glShaderSource;
inline OpenGLFunction< ORB_STRING_LITERAL_32( "glTexSubImage2D" ),            void( GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid* data )  >      glTexSubImage2D;
inline OpenGLFunction< ORB_STRING_LITERAL_32( "glUniform1f" ),                void( GLint location, GLfloat v0 ) >                                                                                                                    glUniform1f;
inline OpenGLFunction< ORB_STRING_LITERAL_32( "glUniform1fv" ),               void( GLint location, GLsizei count, const GLfloat* value ) >                                                                                           glUniform1fv;
inline OpenGLFunction< ORB_STRING_LITERAL_32( "glUniform1i" ),                void( GLint location, GLint v0 ) >                                                                                                                      glUniform1i;
inline OpenGLFunction< ORB_STRING_LITERAL_32( "glUniform1iv" ),               void( GLint location, GLsizei count, const GLint* value ) >                                                                                             glUniform1iv;
inline OpenGLFunction< ORB_STRING_LITERAL_32( "glUniform2f" ),                void( GLint location, GLfloat v0, GLfloat v1 ) >                                                                                                        glUniform2f;
inline OpenGLFunction< ORB_STRING_LITERAL_32( "glUniform2fv" ),               void( GLint location, GLsizei count, const GLfloat* value ) >                                                                                           glUniform2fv;
inline OpenGLFunction< ORB_STRING_LITERAL_32( "glUniform2i" ),                void( GLint location, GLint v0, GLint v1 ) >                                                                                                            glUniform2i;
inline OpenGLFunction< ORB_STRING_LITERAL_32( "glUniform2iv" ),               void( GLint location, GLsizei count, const GLint* value ) >                                                                                             glUniform2iv;
inline OpenGLFunction< ORB_STRING_LITERAL_32( "glUniform3f" ),                void( GLint location, GLfloat v0, GLfloat v1, GLfloat v2 ) >                                                                                            glUniform3f;
inline OpenGLFunction< ORB_STRING_LITERAL_32( "glUniform3fv" ),               void( GLint location, GLsizei count, const GLfloat* value ) >                                                                                           glUniform3fv;
inline OpenGLFunction< ORB_STRING_LITERAL_32( "glUniform3i" ),                void( GLint location, GLint v0, GLint v1, GLint v2 ) >                                                                                                  glUniform3i;
inline OpenGLFunction< ORB_STRING_LITERAL_32( "glUniform3iv" ),               void( GLint location, GLsizei count, const GLint* value ) >                                                                                             glUniform3iv;
inline OpenGLFunction< ORB_STRING_LITERAL_32( "glUniform4f" ),                void( GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3 ) >                                                                                glUniform4f;
inline OpenGLFunction< ORB_STRING_LITERAL_32( "glUniform4fv" ),               void( GLint location, GLsizei count, const GLfloat* value ) >                                                                                           glUniform4fv;
inline OpenGLFunction< ORB_STRING_LITERAL_32( "glUniform4i" ),                void( GLint location, GLint v0, GLint v1, GLint v2, GLint v3 ) >                                                                                        glUniform4i;
inline OpenGLFunction< ORB_STRING_LITERAL_32( "glUniform4iv" ),               void( GLint location, GLsizei count, const GLint* value ) >                                                                                             glUniform4iv;
inline OpenGLFunction< ORB_STRING_LITERAL_32( "glUniformBlockBinding" ),      void( GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding ) >                                                                          glUniformBlockBinding;
inline OpenGLFunction< ORB_STRING_LITERAL_32( "glUnmapBuffer" ),              GLboolean( OpenGLBufferTarget target ) >                                                                                                                glUnmapBuffer;
inline OpenGLFunction< ORB_STRING_LITERAL_32( "glUseProgram" ),               void( GLuint program ) >                                                                                                                                glUseProgram;
inline OpenGLFunction< ORB_STRING_LITERAL_32( "glValidateProgram" ),          void( GLuint program ) >                                                                                                                                glValidateProgram;
inline OpenGLFunction< ORB_STRING_LITERAL_32( "glVertexAttrib1f" ),           void( GLuint index, GLfloat v0 ) >                                                                                                                      glVertexAttrib1f;
inline OpenGLFunction< ORB_STRING_LITERAL_32( "glVertexAttrib1fv" ),          void( GLuint index, const GLfloat* v ) >                                                                                                                glVertexAttrib1fv;
inline OpenGLFunction< ORB_STRING_LITERAL_32( "glVertexAttrib2f" ),           void( GLuint index, GLfloat v0, GLfloat v1 ) >                                                                                                          glVertexAttrib2f;
inline OpenGLFunction< ORB_STRING_LITERAL_32( "glVertexAttrib2fv" ),          void( GLuint index, const GLfloat* v ) >                                                                                                                glVertexAttrib2fv;
inline OpenGLFunction< ORB_STRING_LITERAL_32( "glVertexAttrib3f" ),           void( GLuint index, GLfloat v0, GLfloat v1, GLfloat v2 ) >                                                                                              glVertexAttrib3f;
inline OpenGLFunction< ORB_STRING_LITERAL_32( "glVertexAttrib3fv" ),          void( GLuint index, const GLfloat* v ) >                                                                                                                glVertexAttrib3fv;
inline OpenGLFunction< ORB_STRING_LITERAL_32( "glVertexAttrib4f" ),           void( GLuint index, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3 ) >                                                                                  glVertexAttrib4f;
inline OpenGLFunction< ORB_STRING_LITERAL_32( "glVertexAttrib4fv" ),          void( GLuint index, const GLfloat* v ) >                                                                                                                glVertexAttrib4fv;
inline OpenGLFunction< ORB_STRING_LITERAL_32( "glVertexAttribBinding" ),      void( GLuint attribindex, GLuint bindingindex ) >                                                                                                       glVertexAttribBinding;
inline OpenGLFunction< ORB_STRING_LITERAL_32( "glVertexAttribDivisor" ),      void( GLuint index, GLuint divisor ) >                                                                                                                  glVertexAttribDivisor;
inline OpenGLFunction< ORB_STRING_LITERAL_32( "glVertexAttribFormat" ),       void( GLuint attribindex, GLint size, OpenGLVertexAttribDataType type, GLboolean normalized, GLuint relativeoffset ) >                                  glVertexAttribFormat;
inline OpenGLFunction< ORB_STRING_LITERAL_32( "glVertexAttribI4i" ),          void( GLuint index, GLint v0, GLint v1, GLint v2, GLint v3 ) >                                                                                          glVertexAttribI4i;
inline OpenGLFunction< ORB_STRING_LITERAL_32( "glVertexAttribI4iv" ),         void( GLuint index, const GLint* v ) >                                                                                                                  glVertexAttribI4iv;
inline OpenGLFunction< ORB_STRING_LITERAL_32( "glVertexAttribI4ui" ),         void( GLuint index, GLuint v0, GLuint v1, GLuint v2, GLuint v3 ) >                                                                                      glVertexAttribI4ui;
inline OpenGLFunction< ORB_STRING_LITERAL_32( "glVertexAttribI4uiv" ),        void( GLuint index, const GLuint* v ) >                                                                                                                 glVertexAttribI4uiv;
inline OpenGLFunction< ORB_STRING_LITERAL_32( "glVertexAttribIFormat" ),      void( GLuint attribindex, GLint size, OpenGLVertexAttribDataType type, GLuint relativeoffset ) >                                                        glVertexAttribIFormat;
inline OpenGLFunction< ORB_STRING_LITERAL_32( "glVertexAttribPointer" ),      void( GLuint index, GLint size, OpenGLVertexAttribDataType type, GLboolean normalized, GLsizei stride, const GLvoid* pointer ) >                        glVertexAttribPointer;
inline OpenGLFunction< ORB_STRING_LITERAL_32( "glVertexBindingDivisor" ),     void( GLuint bindingindex, GLuint divisor ) >                                                                                                           glVertexBindingDivisor;

ORB_NAMESPACE_END
