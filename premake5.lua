if (_ACTION == "xcode4") then
	require "xcode"

	local cpplanguagestandards = {["C++11"] = "c++11", ["C++14"] = "c++14", ["C++17"] = "c++1z"}
	premake.override(premake.modules.xcode, "XCBuildConfiguration_CppLanguageStandard", function(base, settings, cfg)
		if cfg.cppdialect then
			settings["CLANG_CXX_LANGUAGE_STANDARD"] = cpplanguagestandards[cfg.cppdialect] or "compiler-default"
		end
	end)
elseif (_ACTION == "codelite") then
	require "codelite"

	premake.override(premake.modules.codelite.project, "environment", function(base, cfg)
		local envs = table.concat(cfg.debugenvs, "\n")
		envs = envs .. string.format("\nLD_LIBRARY_PATH=%s:$LD_LIBRARY_PATH", premake.project.getrelative(cfg.project, cfg.targetdir))

		_p(3, "<Environment EnvVarSetName=\"&lt;Use Default&gt;\" DbgSetName=\"&lt;Use Default&gt;\">")
		_p(4, "<![CDATA[%s]]>", envs)
		_p(3, "</Environment>")
	end)
end

local function get_arch()
	return io.popen("uname -m", "r"):read("*l")
end

local function get_platforms()
	local platforms = { get_arch() }

	-- Add x86 target for x64 Windows builds
	if os.host() == "windows" and platforms[1] == "x86_64" then
		table.insert(platforms, "x86")
	end

	return platforms
end

local function base_config()
	location       ("build/%{_ACTION}/")
	objdir         ("build/%{_ACTION}/%{cfg.platform}/%{cfg.buildcfg}/")
	targetdir      ("build/%{_ACTION}/%{cfg.platform}/%{cfg.buildcfg}/")
	includedirs    {"src/"}
	sysincludedirs {"src/"}
	cppdialect     ("C++17")
	warnings       ("Extra")
	filter{"configurations:Debug"}
	optimize       ("Off")
	symbols        ("On")
	filter{"configurations:Release"}
	defines        {"NDEBUG"}
	optimize       ("Full")
	symbols        ("Off")
	filter{"system:windows"}
	toolset        ("msc")
	filter{"system:not windows"}
	toolset        ("clang")
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
		["windows"] = {"windows", "win32", "wgl", "d3d11"},
		["android"] = {"android", "egl"},
		["linux"]   = {"linux", "x11", "glx"},
		["macosx"]  = {"macos", "cocoa"},
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
	filter{"system:macosx"} files{"src/orbit/" .. lo .. "/**.mm"} filter{}
	filter_system_files()
	group()
	table.insert(modules, name)
end

local sample_index = 1
local function decl_sample(name)
	local id = string.format("%02d", sample_index)
	group("Samples")
	project (id .. "." .. name)
	kind    ("ConsoleApp")
	links   (modules)
	base_config()
	files {
		"src/samples/" .. id .. "/**.cpp",
		"src/samples/" .. id .. "/**.h",
	}
	filter_system_files()
	group()
	sample_index = sample_index + 1
end

workspace      ("Orbit")
platforms      (get_platforms())
configurations {"Debug", "Release"}

-- Engine modules
decl_module("Core")
  filter{"system:macosx"} links{"Cocoa.framework"}
decl_module("Graphics")
  filter{"system:windows"} links{"opengl32", "d3d11", "dxgi"}
  filter{"system:macosx"} links{"Cocoa.framework", "OpenGL.framework"}

-- Samples
decl_sample("Base")
  filter{"system:linux"} links{"X11", "GL"}
decl_sample("Benchmarking")
  filter{"system:linux"} links{"X11", "GL"}
