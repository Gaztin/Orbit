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
	cppdialect     ("C++14")
	warnings       ("Extra")
	filter{"configurations:Debug"}
	optimize       ("Off")
	symbols        ("On")
	filter{"configurations:Release"}
	optimize       ("Full")
	symbols        ("Off")
	filter{}
end

local modules = {}
local function decl_module(name)
	local lo = name:lower()
	local up = name:upper()
	group("Engine")
	project (name)
	kind    ("SharedLib")
	defines {"ORB_BUILD_" .. up}
	base_config()
	files {
		"src/orbit/" .. lo .. ".h",
		"src/orbit/" .. lo .. "/**.cpp",
		"src/orbit/" .. lo .. "/**.h",
	}
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
	group()
	sample_index = sample_index + 1
end

workspace      ("Orbit")
platforms      (get_platforms())
configurations {"Debug", "Release"}

-- Engine modules
decl_module("Core")

-- Samples
decl_sample("Base")
