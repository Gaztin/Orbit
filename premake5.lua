require 'third_party/premake-android-studio'

OUTDIR = 'build/%{_ACTION}/%{cfg.platform}/%{cfg.buildcfg}/'
FRAMEWORK_NAME = '00-Framework'

-- Allow Objective C++ files on macOS and iOS
premake.api.addAllowed( 'language', 'ObjCpp' )

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
	minsdkversion( '23' )
	maxsdkversion( '28' )
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
	appid( 'com.gaztin.orbit.libs.' .. name:lower() )
	base_config()
	files {
		'src/Orbit/' .. name .. '/**.cpp',
		'src/Orbit/' .. name .. '/**.h',
	}

	filter { 'toolset:msc' }
		defines { '_CRT_SECURE_NO_WARNINGS' }
	filter { 'system:macosx or ios' }
		files { 'src/Orbit/' .. name .. '/**.mm' }
	filter { 'system:macosx or ios', 'files:**.cpp' }
		language( 'ObjCpp' )
	filter { 'system:android' }
		includedirs { ANDROID_NATIVE_APP_GLUE_DIR }
	filter { }

	project()
	group()

	table.insert( modules, name )
end

local function decl_framework()
	group( 'Samples' )
	project( FRAMEWORK_NAME )
	kind( 'StaticLib' )
	appid( 'com.gaztin.orbit.libs.framework')
	base_config()
	files {
		string.format( 'src/Samples/%s/**', FRAMEWORK_NAME ),
	}

	filter { 'system:macosx or ios', 'files:**.cpp' }
		language( 'ObjCpp' )
	filter { }

	project()
	group()
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
	appid( 'com.gaztin.orbit.samples.' .. name:lower() )
	base_config()
	files {
		'src/Samples/' .. fullname .. '/*.cpp',
		'src/Samples/' .. fullname .. '/*.h',
	}
	includedirs {
		string.format( 'src/Samples/%s/', FRAMEWORK_NAME ),
	}
	links {
		FRAMEWORK_NAME,
	}

	filter { 'system:linux' }
		linkoptions { '-Wl,-rpath=\\$$ORIGIN' }
	filter { 'system:macosx or ios', 'files:**.cpp' }
		language( 'ObjCpp' )
	filter { 'system:ios' }
		files { 'res/Info.plist', 'assets' }
	filter { 'system:android' }
		files { 'src/Samples/' .. fullname .. '/Android/**', 'res/**' }
	filter { 'system:android' }
		assetdirs { 'assets/' }
	filter { }

	project()
	group()

	table.insert( samples, fullname )
end

local workspace_name = 'Orbit'

workspace( workspace_name )
	platforms( get_platforms() )
	configurations { 'Debug', 'Release' }
	gradleversion( '3.1.4' )

decl_module( 'Core' )
	filter { 'system:macosx' }
		links { 'Cocoa.framework', 'Carbon.framework' }
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

decl_framework()
decl_sample( 'Triangle' )
decl_sample( 'Cube' )

workspace( workspace_name )
	startproject( samples[ 1 ] )
