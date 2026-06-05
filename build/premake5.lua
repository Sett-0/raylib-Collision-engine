workplaceName = "Testing raylib"
projectName   = "raylib-start"

workspace (workplaceName)
	configurations { "Debug", "Release" }
	platforms { "x64" }
	location "../"
    targetdir "../bin/%{cfg.buildcfg}"
	
	filter "configurations:Debug"
		defines { "DEBUG" }
		symbols "On"
	filter "configurations:Release"
		defines { "NDEBUG" }
		optimize "On"
	filter {}
	
project (projectName)
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	
	files { "../simple_balls.cpp" }
	includedirs { "../include" }
	libdirs { "../lib" }
	links { "raylib", "gdi32", "winmm" }

-- Creating 'make run' command
local wks_gen = premake.modules.gmake.generate_workspace
premake.modules.gmake.generate_workspace = function(wks)
	-- Do make as usual
    wks_gen(wks)
    
	-- Add run command 
    premake.w("\nrun: all")
    premake.w("@bin/$(if $(findstring release,$(config)),Release,Debug)/" .. projectName .. ".exe")
end