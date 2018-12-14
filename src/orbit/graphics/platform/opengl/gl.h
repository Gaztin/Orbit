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
//#include <X11/Xlib.h>
#include <GL/glx.h>
#include <GL/gl.h>
#elif defined(ORB_OS_MACOS)
#include <OpenGL/gl.h>
#elif defined(ORB_OS_ANDROID)
#include <EGL/egl.h>
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

enum class symbolic_constant : GLenum
{
	AccumAlphaBits                   = 0x0d5b,
	AccumBlueBits                    = 0x0d5a,
	AccumClearValue                  = 0x0b80,
	AccumGreenBits                   = 0x0d59,
	AccumRedBits                     = 0x0d58,
	ActiveTexture                    = 0x84e0,
	AliasedPointSizeRange            = 0x846d,
	AliasedLineWidthRange            = 0x846e,
	AlphaBias                        = 0x0d1d,
	AlphaBits                        = 0x0d55,
	AlphaScale                       = 0x0d1c,
	AlphaTest                        = 0x0bc0,
	AlphaTestFunc                    = 0x0bc1,
	AlphaTestRef                     = 0x0bc2,
	ArrayBufferBinding               = 0x8894,
	AttribStackDepth                 = 0x0bb0,
	AutoNormal                       = 0x0d80,
	AuxBuffers                       = 0x0c00,
	Blend                            = 0x0be2,
	BlendColor                       = 0x8005,
	BlendDstAlpha                    = 0x80ca,
	BlendDstRgb                      = 0x80c8,
	BlendEquationRgb                 = 0x8009,
	BlendEquationAlpha               = 0x883d,
	BlendSrcAlpha                    = 0x80cb,
	BlendSrcRgb                      = 0x80c9,
	BlueBias                         = 0x0d1b,
	BlueBits                         = 0x0d54,
	BlueScale                        = 0x0d1a,
	ClientActiveTexture              = 0x84e1,
	ClientAttribStackDepth           = 0x0bb1,
	ClipPlane0                       = 0x3000,
	ClipPlane1                       = 0x3001,
	ClipPlane2                       = 0x3002,
	ClipPlane3                       = 0x3003,
	ClipPlane4                       = 0x3004,
	ClipPlane5                       = 0x3005,
	ColorArray                       = 0x8076,
	ColorArrayBufferBinding          = 0x8898,
	ColorArraySize                   = 0x8081,
	ColorArrayStride                 = 0x8083,
	ColorArrayType                   = 0x8082,
	ColorClearValue                  = 0x0c22,
	ColorLogicOp                     = 0x0bf2,
	ColorMaterial                    = 0x0b57,
	ColorMaterialFace                = 0x0b55,
	ColorMaterialParameter           = 0x0b56,
	ColorMatrix                      = 0x80b1,
	ColorMatrixStackDepth            = 0x80b2,
	ColorSum                         = 0x8458,
	ColorTable                       = 0x80d0,
	ColorWritemask                   = 0x0c23,
	CompressedTextureFormats         = 0x86a3,
	Convolution1d                    = 0x8010,
	Convolution2d                    = 0x8011,
	CullFace                         = 0x0b44,
	CullFaceMode                     = 0x0b45,
	CurrentColor                     = 0x0b00,
	CurrentFogCoord                  = 0x8453,
	CurrentIndex                     = 0x0b01,
	CurrentNormal                    = 0x0b02,
	CurrentProgram                   = 0x8b8d,
	CurrentRasterColor               = 0x0b04,
	CurrentRasterDistance            = 0x0b09,
	CurrentRasterIndex               = 0x0b05,
	CurrentRasterPosition            = 0x0b07,
	CurrentRasterPositionValid       = 0x0b08,
	CurrentRasterSecondaryColor      = 0x845f,
	CurrentRasterTextureCoords       = 0x0b06,
	CurrentSecondaryColor            = 0x8459,
	CurrentTextureCoords             = 0x0b03,
	DepthBias                        = 0x0d1f,
	DepthBits                        = 0x0d56,
	DepthClearValue                  = 0x0b73,
	DepthFunc                        = 0x0b74,
	DepthRange                       = 0x0b70,
	DepthScale                       = 0x0d1e,
	DepthTest                        = 0x0b71,
	DepthWritemask                   = 0x0b72,
	Dither                           = 0x0bd0,
	Doublebuffer                     = 0x0c32,
	DrawBuffer                       = 0x0c01,
	DrawBuffer0                      = 0x8825,
	DrawBuffer1                      = 0x8826,
	DrawBuffer2                      = 0x8827,
	DrawBuffer3                      = 0x8828,
	DrawBuffer4                      = 0x8829,
	DrawBuffer5                      = 0x882a,
	DrawBuffer6                      = 0x882b,
	DrawBuffer7                      = 0x882c,
	DrawBuffer8                      = 0x882d,
	DrawBuffer9                      = 0x882e,
	DrawBuffer10                     = 0x882f,
	DrawBuffer11                     = 0x8830,
	DrawBuffer12                     = 0x8831,
	DrawBuffer13                     = 0x8832,
	DrawBuffer14                     = 0x8833,
	DrawBuffer15                     = 0x8834,
	EdgeFlag                         = 0x0b43,
	EdgeFlagArray                    = 0x8079,
	EdgeFlagArrayBufferBinding       = 0x889b,
	EdgeFlagArrayStride              = 0x808c,
	ElementArrayBufferBinding        = 0x8895,
	FeedbackBufferSize               = 0x0df1,
	FeedbackBufferType               = 0x0df2,
	Fog                              = 0x0b60,
	FogCoordArray                    = 0x8457,
	FogCoordArrayBufferBinding       = 0x889d,
	FogCoordArrayStride              = 0x8455,
	FogCoordArrayType                = 0x8454,
	FogCoordSrc                      = 0x8450,
	FogColor                         = 0x0b66,
	FogDensity                       = 0x0b62,
	FogEnd                           = 0x0b64,
	FogHint                          = 0x0c54,
	FogIndex                         = 0x0b61,
	FogMode                          = 0x0b65,
	FogStart                         = 0x0b63,
	FragmentShaderDerivativeHint     = 0x8b8b,
	FrontFace                        = 0x0b46,
	GenerateMipmapHint               = 0x8192,
	GreenBias                        = 0x0d19,
	GreenBits                        = 0x0d53,
	GreenScale                       = 0x0d18,
	Histogram                        = 0x8024,
	IndexArray                       = 0x8077,
	IndexArrayBufferBinding          = 0x8899,
	IndexArrayStride                 = 0x8086,
	IndexArrayType                   = 0x8085,
	IndexBits                        = 0x0d51,
	IndexClearValue                  = 0x0c20,
	IndexLogicOp                     = 0x0bf1,
	IndexMode                        = 0x0c30,
	IndexOffset                      = 0x0d13,
	IndexShift                       = 0x0d12,
	IndexWritemask                   = 0x0c21,
	Light0                           = 0x4000,
	Lighting                         = 0x0b50,
	LightModelAmbient                = 0x0b53,
	LightModelColorControl           = 0x81f8,
	LightModelLocalViewer            = 0x0b51,
	LightModelTwoSide                = 0x0b52,
	LineSmooth                       = 0x0b20,
	LineSmoothHint                   = 0x0c52,
	LineStipple                      = 0x0b24,
	LineStipplePattern               = 0x0b25,
	LineStippleRepeat                = 0x0b26,
	LineWidth                        = 0x0b21,
	LineWidthGranularity             = 0x0b23,
	LineWidthRange                   = 0x0b22,
	ListBase                         = 0x0b32,
	ListIndex                        = 0x0b33,
	ListMode                         = 0x0b30,
	LogicOpMode                      = 0x0bf0,
	Map1Color4                       = 0x0d90,
	Map1GridDomain                   = 0x0dd0,
	Map1GridSegments                 = 0x0dd1,
	Map1Index                        = 0x0d91,
	Map1Normal                       = 0x0d92,
	Map1TextureCoord1                = 0x0d93,
	Map1TextureCoord2                = 0x0d94,
	Map1TextureCoord3                = 0x0d95,
	Map1TextureCoord4                = 0x0d96,
	Map1Vertex3                      = 0x0d97,
	Map1Vertex4                      = 0x0d98,
	Map2Color4                       = 0x0db0,
	Map2GridDomain                   = 0x0dd2,
	Map2GridSegments                 = 0x0dd3,
	Map2Index                        = 0x0db1,
	Map2Normal                       = 0x0db2,
	Map2TextureCoord1                = 0x0db3,
	Map2TextureCoord2                = 0x0db4,
	Map2TextureCoord3                = 0x0db5,
	Map2TextureCoord4                = 0x0db6,
	Map2Vertex3                      = 0x0db7,
	Map2Vertex4                      = 0x0db8,
	MapColor                         = 0x0d10,
	MapStencil                       = 0x0d11,
	MatrixMode                       = 0x0ba0,
	Max3dTextureSize                 = 0x8073,
	MaxClientAttribStackDepth        = 0x0d3b,
	MaxAttribStackDepth              = 0x0d35,
	MaxClipPlanes                    = 0x0d32,
	MaxColorMatrixStackDepth         = 0x80b3,
	MaxCombinedTextureImageUnits     = 0x8b4d,
	MaxCubeMapTextureSize            = 0x851c,
	MaxDrawBuffers                   = 0x8824,
	MaxElementsIndices               = 0x80e9,
	MaxElementsVertices              = 0x80e8,
	MaxEvalOrder                     = 0x0d30,
	MaxFragmentUniformComponents     = 0x8b49,
	MaxLights                        = 0x0d31,
	MaxListNesting                   = 0x0b31,
	MaxModelviewStackDepth           = 0x0d36,
	MaxNameStackDepth                = 0x0d37,
	MaxPixelMapTable                 = 0x0d34,
	MaxProjectionStackDepth          = 0x0d38,
	MaxTextureCoords                 = 0x8871,
	MaxTextureImageUnits             = 0x8872,
	MaxTextureLodBias                = 0x84fd,
	MaxTextureSize                   = 0x0d33,
	MaxTextureStackDepth             = 0x0d39,
	MaxTextureUnits                  = 0x84e2,
	MaxVaryingFloats                 = 0x8b4b,
	MaxVertexAttribs                 = 0x8869,
	MaxVertexTextureImageUnits       = 0x8b4c,
	MaxVertexUniformComponents       = 0x8b4a,
	MaxViewportDims                  = 0x0d3a,
	Minmax                           = 0x802e,
	ModelviewMatrix                  = 0x0ba6,
	ModelviewStackDepth              = 0x0ba3,
	NameStackDepth                   = 0x0d70,
	NormalArray                      = 0x8075,
	NormalArrayBufferBinding         = 0x8897,
	NormalArrayStride                = 0x807f,
	NormalArrayType                  = 0x807e,
	Normalize                        = 0x0ba1,
	NumCompressedTextureFormats      = 0x86a2,
	PackAlignment                    = 0x0d05,
	PackImageHeight                  = 0x806c,
	PackLsbFirst                     = 0x0d01,
	PackRowLength                    = 0x0d02,
	PackSkipImages                   = 0x806b,
	PackSkipPixels                   = 0x0d04,
	PackSkipRows                     = 0x0d03,
	PackSwapBytes                    = 0x0d00,
	PerspectiveCorrectionHint        = 0x0c50,
	PixelMapAToASize                 = 0x0cb9,
	PixelMapBToBSize                 = 0x0cb8,
	PixelMapGToGSize                 = 0x0cb7,
	PixelMapIToASize                 = 0x0cb5,
	PixelMapIToBSize                 = 0x0cb4,
	PixelMapIToGSize                 = 0x0cb3,
	PixelMapIToISize                 = 0x0cb0,
	PixelMapIToRSize                 = 0x0cb2,
	PixelMapRToRSize                 = 0x0cb6,
	PixelMapSToSSize                 = 0x0cb1,
	PixelPackBufferBinding           = 0x88ed,
	PixelUnpackBufferBinding         = 0x88ef,
	PointDistanceAttenuation         = 0x8129,
	PointFadeThresholdSize           = 0x8128,
	PointSize                        = 0x0b11,
	PointSizeGranularity             = 0x0b13,
	PointSizeMax                     = 0x8127,
	PointSizeMin                     = 0x8126,
	PointSizeRange                   = 0x0b12,
	PointSmooth                      = 0x0b10,
	PointSmoothHint                  = 0x0c51,
	PointSprite                      = 0x8861,
	PolygonMode                      = 0x0b40,
	PolygonOffsetFactor              = 0x8038,
	PolygonOffsetUnits               = 0x2a00,
	PolygonOffsetFill                = 0x8037,
	PolygonOffsetLine                = 0x2a02,
	PolygonOffsetPoint               = 0x2a01,
	PolygonSmooth                    = 0x0b41,
	PolygonSmoothHint                = 0x0c53,
	PolygonStipple                   = 0x0b42,
	PostColorMatrixColorTable        = 0x80d2,
	PostColorMatrixRedBias           = 0x80b8,
	PostColorMatrixGreenBias         = 0x80b9,
	PostColorMatrixBlueBias          = 0x80ba,
	PostColorMatrixAlphaBias         = 0x80bb,
	PostColorMatrixRedScale          = 0x80b4,
	PostColorMatrixGreenScale        = 0x80b5,
	PostColorMatrixBlueScale         = 0x80b5,
	PostColorMatrixAlphaScale        = 0x80b7,
	PostConvolutionColorTable        = 0x80d1,
	PostConvolutionRedBias           = 0x8020,
	PostConvolutionGreenBias         = 0x8021,
	PostConvolutionBlueBias          = 0x8022,
	PostConvolutionAlphaBias         = 0x8023,
	PostConvolutionRedScale          = 0x801c,
	PostConvolutionGreenScale        = 0x801d,
	PostConvolutionBlueScale         = 0x801e,
	PostConvolutionAlphaScale        = 0x801f,
	ProjectionMatrix                 = 0x0ba7,
	ProjectionStackDepth             = 0x0ba4,
	ReadBuffer                       = 0x0c02,
	RedBias                          = 0x0d15,
	RedBits                          = 0x0d52,
	RedScale                         = 0x0d14,
	RenderMode                       = 0x0c40,
	RescaleNormal                    = 0x803a,
	RgbaMode                         = 0x0c31,
	SampleBuffers                    = 0x80a8,
	SampleCoverageValue              = 0x80aa,
	SampleCoverageInvert             = 0x80ab,
	Samples                          = 0x80a9,
	ScissorBox                       = 0x0c10,
	ScissorTest                      = 0x0c11,
	SecondaryColorArray              = 0x845e,
	SecondaryColorArrayBufferBinding = 0x889c,
	SecondaryColorArraySize          = 0x845a,
	SecondaryColorArrayStride        = 0x845c,
	SecondaryColorArrayType          = 0x845b,
	SelectionBufferSize              = 0x0df4,
	Separable2d                      = 0x8012,
	ShadeModel                       = 0x0b54,
	SmoothLineWidthRange             = 0x0b22,
	SmoothLineWidthGranularity       = 0x0b23,
	SmoothPointSizeRange             = 0x0b12,
	SmoothPointSizeGranularity       = 0x0b13,
	StencilBackFail                  = 0x8801,
	StencilBackFunc                  = 0x8800,
	StencilBackPassDepthFail         = 0x8802,
	StencilBackPassDepthPass         = 0x8803,
	StencilBackRef                   = 0x8ca3,
	StencilBackValueMask             = 0x8ca4,
	StencilBackWritemask             = 0x8ca5,
	StencilBits                      = 0x0d57,
	StencilClearValue                = 0x0b91,
	StencilFail                      = 0x0b94,
	StencilFunc                      = 0x0b92,
	StencilPassDepthFail             = 0x0b95,
	StencilPassDepthPass             = 0x0b96,
	StencilRef                       = 0x0b97,
	StencilTest                      = 0x0b90,
	StencilValueMask                 = 0x0b93,
	StencilWritemask                 = 0x0b98,
	Stereo                           = 0x0c33,
	SubpixelBits                     = 0x0d50,
	Texture1d                        = 0x0de0,
	TextureBinding1d                 = 0x8068,
	Texture2d                        = 0x0de1,
	TextureBinding2d                 = 0x8069,
	Texture3d                        = 0x806f,
	TextureBinding3d                 = 0x806a,
	TextureBindingCubeMap            = 0x8514,
	TextureCompressionHint           = 0x84ef,
	TextureCoordArray                = 0x8078,
	TextureCoordArrayBufferBinding   = 0x889a,
	TextureCoordArraySize            = 0x8088,
	TextureCoordArrayStride          = 0x808a,
	TextureCoordArrayType            = 0x8089,
	TextureCubeMap                   = 0x8513,
	TextureGenQ                      = 0x0c63,
	TextureGenR                      = 0x0c62,
	TextureGenS                      = 0x0c60,
	TextureGenT                      = 0x0c61,
	TextureMatrix                    = 0x0ba8,
	TextureStackDepth                = 0x0ba5,
	TransposeColorMatrix             = 0x84e6,
	TransposeModelviewMatrix         = 0x84e3,
	TransposeProjectionMatrix        = 0x84e4,
	TransposeTextureMatrix           = 0x84e5,
	UnpackAlignment                  = 0x0cf5,
	UnpackImageHeight                = 0x806e,
	UnpackLsbFirst                   = 0x0cf1,
	UnpackRowLength                  = 0x0cf2,
	UnpackSkipImages                 = 0x806d,
	UnpackSkipPixels                 = 0x0cf4,
	UnpackSkipRows                   = 0x0cf3,
	UnpackSwapBytes                  = 0x0cf0,
	VertexArray                      = 0x8074,
	VertexArrayBufferBinding         = 0x8896,
	VertexArraySize                  = 0x807a,
	VertexArrayStride                = 0x807c,
	VertexArrayType                  = 0x807b,
	VertexProgramPointSize           = 0x8642,
	VertexProgramTwoSide             = 0x8643,
	Viewport                         = 0x0ba2,
	ZoomX                            = 0x0d16,
	ZoomY                            = 0x0d17,
};

#if defined(ORB_OS_WINDOWS)
#define ORB_GL_CALL __stdcall
#else
#define ORB_GL_CALL
#endif

struct functions
{
	static inline void get_booleanv(symbolic_constant pname, GLboolean* params) { return glGetBooleanv(static_cast<GLenum>(pname), params); }
	static inline void get_doublev(symbolic_constant pname, GLdouble* params) { return glGetDoublev(static_cast<GLenum>(pname), params); }
	static inline void get_floatv(symbolic_constant pname, GLfloat* params) { return glGetFloatv(static_cast<GLenum>(pname), params); }
	static inline void get_integerv(symbolic_constant pname, GLint* params) { return glGetIntegerv(static_cast<GLenum>(pname), params); }

	void (ORB_GL_CALL *bind_buffer)(buffer_target target, GLuint buffer);
	void (ORB_GL_CALL *buffer_data)(buffer_target target, GLsizeiptr size, const GLvoid* data, buffer_usage usage);
	void (ORB_GL_CALL *buffer_sub_data)(buffer_target target, GLintptr offset, GLsizeiptr size, const GLvoid* data);
	void (ORB_GL_CALL *delete_buffers)(GLsizei n, const GLuint* buffers);
	void (ORB_GL_CALL *disable_vertex_attrib_array)(GLuint index);
	void (ORB_GL_CALL *draw_arrays)(draw_mode mode, GLint first, GLsizei count);
	void (ORB_GL_CALL *draw_elements)(draw_mode, GLsizei count, index_type type, const GLvoid* indices);
	void (ORB_GL_CALL *enable_vertex_attrib_array)(GLuint index);
	void (ORB_GL_CALL *gen_buffers)(GLsizei n, GLuint* buffers);
	void (ORB_GL_CALL *get_buffer_parameteriv)(buffer_target target, buffer_param value, GLint* data);
	void (ORB_GL_CALL *get_buffer_pointerv)(buffer_target target, buffer_pointer_param pname, GLvoid** params);
	void (ORB_GL_CALL *get_vertex_attribdv)(GLuint index, vertex_attrib_array_param pname, GLdouble* params);
	void (ORB_GL_CALL *get_vertex_attribfv)(GLuint index, vertex_attrib_array_param pname, GLfloat* params);
	void (ORB_GL_CALL *get_vertex_attribiv)(GLuint index, vertex_attrib_array_param pname, GLint* params);
	void (ORB_GL_CALL *get_vertex_attrib_pointerv)(GLuint index, vertex_attrib_array_pointer_param pname, GLvoid** pointer);
	GLboolean (ORB_GL_CALL *is_buffer)(GLuint buffer);
	void (ORB_GL_CALL *vertex_attrib1f)(GLuint index, GLfloat v0);
	void (ORB_GL_CALL *vertex_attrib2f)(GLuint index, GLfloat v0, GLfloat v1);
	void (ORB_GL_CALL *vertex_attrib3f)(GLuint index, GLfloat v0, GLfloat v1, GLfloat v2);
	void (ORB_GL_CALL *vertex_attrib4f)(GLuint index, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
	void (ORB_GL_CALL *vertex_attrib_pointer)(GLuint index, GLint size, vertex_attrib_data_type type, GLboolean normalized, GLsizei stride, const GLvoid* pointer);
};

extern ORB_API_GRAPHICS functions load_functions();

namespace platform
{

extern ORB_API_GRAPHICS void* get_proc_address(std::string_view name);

}

}
}

#endif
