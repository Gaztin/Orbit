OUTDIR = "build/%{_ACTION}/%{cfg.platform}/%{cfg.buildcfg}/"

if _TARGET_OS == "macosx" then
	newoption {
		trigger = "ios",
		description = "Target iOS"
	}
	if _OPTIONS["ios"] then
		_TARGET_OS = "ios"
	end
end

local function get_arch()
	return io.popen("uname -m", "r"):read("*l")
end

local function get_app_kind()
	if (_TARGET_OS == "ios") then
		return "WindowedApp"
	else
		return "ConsoleApp"
	end
end

local function get_platforms()

	-- Add x86 target for x64 Windows builds
	if os.host() == "windows" and get_arch() == "x86_64" then
		return { "x64", "x86" }
	else
		return { get_arch() }
	end
end

local function base_config()
	location          ("build/%{_ACTION}/")
	objdir            (OUTDIR)
	targetdir         (OUTDIR)
	debugdir          ("assets/")
	includedirs       {"src/"}
	sysincludedirs    {"src/"}
	cppdialect        ("C++17")
	warnings          ("Extra")
	rtti              ("Off")
	exceptionhandling ("Off")
	filter{"configurations:Debug"}
	optimize          ("Off")
	symbols           ("On")
	filter{"configurations:Release"}
	defines           {"NDEBUG"}
	optimize          ("Full")
	symbols           ("Off")
	filter{"system:windows"}
	toolset           ("msc")
	defines           {"NOMINMAX"}
	filter{"system:not windows"}
	toolset           ("gcc")
	filter{"system:linux"}
	debugenvs         {"LD_LIBRARY_PATH=$LD_LIBRARY_PATH:../%{OUTDIR}"}
	filter{}
end

local function foreach_system(functor)
	local field = premake.field.get("system")
	for i = 1, #field.allowed do
		functor(field.allowed[i])
	end
	functor("android")
end

local function foreach_system_keywords(os, functor)
	local keywords = {
		["windows"] = {"win32",   "desktop",          "gl", "d3d11"},
		["linux"]   = {"linux",   "desktop", "posix", "gl"},
		["macosx"]  = {"macos",   "desktop", "posix", "gl"},
		["android"] = {"android", "mobile",  "gl"},
		["ios"]     = {"ios",     "mobile",  "gl"},
	}
	if keywords[os] == nil then
		return
	end
	for i = 1, #keywords[os] do
		functor(keywords[os][i])
	end
end

local function filter_system_files()
	foreach_system(function(os)
		-- Exclude files containing keywords from other systems
		filter{"system:not " .. os}
		foreach_system_keywords(os, function(keyword)
			-- Keywords may appear in multiple systems
			local target_has_keyword = false
			foreach_system_keywords(_TARGET_OS, function(keyword2)
				if (keyword2 == keyword) then
					target_has_keyword = true
				end
			end)
			if not target_has_keyword then
				removefiles {
					"src/**" .. keyword .. "_**",
					"src/**_" .. keyword .. "**",
					"src/**/" .. keyword .. ".*",
				}
			end
		end)
	end)
end

local modules = {}
local function decl_module(name)
	local lo = name:lower()
	local up = name:upper()
	group("Engine")
	project (name)
	kind    ("SharedLib")
	defines {"ORB_BUILD", "ORB_BUILD_" .. up}
	links   (modules)
	base_config()
	files {
		"src/orbit.h",
		"src/orbit/" .. lo .. ".h",
		"src/orbit/" .. lo .. "/**.cpp",
		"src/orbit/" .. lo .. "/**.h",
	}
	filter{"system:macosx or ios"} files{"src/orbit/" .. lo .. "/**.mm"} filter{}
	filter_system_files()
	group()
	table.insert(modules, name)
end

local sample_index = 1
local function decl_sample(name)
	local id = string.format("%02d", sample_index)
	group("Samples")
	project (id .. "." .. name)
	kind    (get_app_kind())
	links   (modules)
	xcodebuildresources("assets")
	base_config()
	files {
		"src/samples/" .. id .. "/**.cpp",
		"src/samples/" .. id .. "/**.h",
	}
	filter{"system:ios"} files{"res/Info.plist", "assets"} filter{}
	filter_system_files()
	group()
	sample_index = sample_index + 1
end

workspace      ("Orbit")
platforms      (get_platforms())
configurations {"Debug", "Release"}

-- Engine modules
decl_module("Core")
  filter{"system:macosx"}
    links{"Cocoa.framework"}
  filter{"system:ios"}
    links{"UIKit.framework", "QuartzCore.framework"}
decl_module("Graphics")
  filter{"system:windows"}
    links{"opengl32", "d3d11", "dxgi", "D3DCompiler"}
  filter{"system:linux"}
    links{"X11", "GL"}
  filter{"system:macosx"}
    links{"Cocoa.framework", "OpenGL.framework"}
    defines{"GL_SILENCE_DEPRECATION"}
  filter{"system:ios"}
    links{"UIKit.framework", "GLKit.framework", "OpenGLES.framework"}
    defines{"GLES_SILENCE_DEPRECATION"}

-- Samples
decl_sample("Base")
  filter{"system:windows"}
    defines{"_USE_MATH_DEFINES"}
decl_sample("Benchmarking")
