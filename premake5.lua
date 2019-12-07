require 'third_party/premake-android-studio/android_studio'

OUTDIR = 'build/%{_ACTION}/%{cfg.platform}/%{cfg.buildcfg}/'
ANDROID_NDK = '${ANDROID_NDK}'
ANDROID_NATIVE_APP_GLUE_DIR = ANDROID_NDK .. '/sources/android/native_app_glue'

-- Allow Objective C++ files on macOS and iOS
premake.api.addAllowed( 'language', 'ObjCpp' )

premake.field.override( 'file', 'store', function( base, f, current, value )
	-- '${ANDROID_NDK}' is not resolved until it is passed into the build environment.
	-- Which means that the file doesn't exist as far as premake knows.
	if( value:startswith( ANDROID_NDK ) ) then
		return value
	else
		return base( f, current, value )
	end
end )

-- Set system to android
if( _ACTION == 'android-studio' ) then
	_TARGET_OS = 'android'
	system( 'android' )
end

if( _TARGET_OS == 'macosx' ) then
	newoption {
		trigger = 'ios',
		description = 'Target iOS'
	}
	if( _OPTIONS[ 'ios' ] ) then
		_TARGET_OS = 'ios'
	end
end

local function get_arch()
	return io.popen( 'uname -m', 'r' ):read( '*l' )
end

local function get_platforms()
	-- Add x86 target for x64 Windows builds
	if( _ACTION == 'android-studio' ) then
		return { }
	elseif( os.host() == 'windows' and get_arch() == 'x86_64' ) then
		return { 'x64', 'x86' }
	else
		return { get_arch() }
	end
end

local function base_config()
	location( 'build/%{_ACTION}/' )
	objdir( OUTDIR )
	targetdir( OUTDIR )
	debugdir( 'assets' )
	cppdialect( 'C++17' )
	warnings( 'Extra' )
	rtti( 'Off' )
	exceptionhandling( 'Off' )
	includedirs { 'src/' }
	sysincludedirs { 'src/' }
	flags { 'MultiProcessorCompile' }

	filter { 'configurations:Debug' }
		optimize( 'Off' )
		symbols( 'On' )
	filter { 'configurations:Release' }
		optimize( 'Full' )
		symbols( 'Off' )
		defines { 'NDEBUG' }
	filter { 'system:windows' }
		toolset( 'msc' )
		defines { 'NOMINMAX' }
	filter { 'system:not windows' }
		toolset( 'gcc' )
	filter { 'system:linux' }
		debugenvs { 'LD_LIBRARY_PATH=$LD_LIBRARY_PATH:../%{OUTDIR}' }
	filter { 'system:android' }
		androidabis { 'armeabi-v7a', 'arm64-v8a', 'x86', 'x86_64' }
		buildoptions { '-Wfatal-errors' }
	filter{ }
end

local function foreach_system( functor )
	local field = premake.field.get( 'system' )
	for i=1,#field.allowed do
		functor( field.allowed[ i ] )
	end
	functor( 'android' )
end

local modules = { }
local function decl_module( name )
	local up = name:upper()
	group( 'Engine' )
	project( name )
	kind( 'SharedLib' )
	defines { 'ORB_BUILD', 'ORB_BUILD_' .. up }
	links( modules )
	base_config()
	files {
		'src/Orbit/' .. name .. '/**.cpp',
		'src/Orbit/' .. name .. '/**.h',
	}

	filter { 'toolset:msc' }
		defines { '_CRT_SECURE_NO_WARNINGS' }
	filter { 'system:macosx or ios', 'files:**' }
		language( 'ObjCpp' )
	filter { 'system:android' }
		includedirs { ANDROID_NATIVE_APP_GLUE_DIR }
	filter { 'action:android-studio' }
		removefiles { '**.h' }
	filter { }

	group()
	table.insert( modules, name )
end

local samples = { }
local function decl_sample( name )
	local id = string.format( '%02d', 1 + #samples )
	local fullname = id .. '-' .. name
	group( 'Samples' )
	project( fullname )
	kind( 'WindowedApp' )
	links( modules )
	xcodebuildresources( 'assets' )
	base_config()
	files {
		'src/Samples/' .. fullname .. '/*.cpp',
		'src/Samples/' .. fullname .. '/*.h',
	}

	filter { 'system:linux' }
		linkoptions { '-Wl,-rpath=\\$$ORIGIN' }
	filter { 'system:ios' }
		files { 'res/Info.plist', 'assets' }
	filter { 'system:android' }
		files { 'src/Samples/' .. fullname .. '/Android/**', 'res/**', ANDROID_NATIVE_APP_GLUE_DIR .. '/android_native_app_glue.c' }
	filter { 'system:android' }
		assetdirs { 'assets/' }
	filter { 'action:android-studio' }
		removefiles { '**.h' }
	filter { }

	group()
	table.insert( samples, fullname )
end

local workspace_name = 'Orbit'

workspace( workspace_name )
	platforms( get_platforms() )
	configurations { 'Debug', 'Release' }
	gradleversion( 'com.android.tools.build:gradle:3.1.4' )

decl_module( 'Core' )
	filter { 'system:macosx' }
		links { 'Cocoa.framework' }
	filter { 'system:android' }
		links { 'log', 'android' }
	filter { 'system:ios' }
		links { 'UIKit.framework', 'QuartzCore.framework' }
	filter { }

decl_module( 'Math' )

decl_module( 'Graphics' )
	filter { 'system:windows' }
		links { 'opengl32', 'd3d11', 'dxgi', 'D3DCompiler' }
	filter { 'system:linux' }
		links { 'X11', 'GL' }
	filter { 'system:macosx' }
		links { 'Cocoa.framework', 'OpenGL.framework' }
		defines { 'GL_SILENCE_DEPRECATION' }
	filter { 'system:android' }
		links { 'android', 'EGL', 'GLESv1_CM', 'GLESv2' }
	filter { 'system:ios' }
		links { 'UIKit.framework', 'GLKit.framework', 'OpenGLES.framework' }
		defines { 'GLES_SILENCE_DEPRECATION' }
	filter { }

decl_sample( 'Triangle' )
decl_sample( 'Cube' )

workspace( workspace_name )
	startproject( samples[ 1 ] )
