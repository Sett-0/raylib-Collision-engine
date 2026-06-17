workplaceName = "Testing-raylib"
projectName   = "I-am-ballin-it"

workspace (workplaceName)
	configurations { "Debug", "Release" }
	architecture (os.hostarch())
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
	
	filter { "system:windows", "configurations:Release", "action:gmake*" }
		kind "WindowedApp"
	
	filter {}
	
	language "C++"
	cppdialect "C++17"
	
	staticruntime "On"
	linkoptions { "-static" }
			
	files { "../simple_balls.cpp" }
	includedirs { "../include" }
	libdirs { "../lib" }
	links { "raylib" }
	
	filter "system:windows"
		defines{ "_WIN32" }
		links { "gdi32", "winmm" }

	filter "system:linux"
		links { "pthread", "m", "dl", "rt", "X11" }
	
	filter {}

-- Creating 'make run' command and fixing bash pathing issue
local wks_gen = premake.modules.gmake.generate_workspace
premake.modules.gmake.generate_workspace = function(wks)
	-- Do make as usual
    wks_gen(wks)
    
	-- Add run command 
    premake.w("\nrun: all")
    premake.w("@bin/$(if $(findstring release,$(config)),Release,Debug)/" .. projectName .. ".exe")
	
	-- Adding double quotes to $(MAKE) to stop bash from breaking if make filepath contains spaces
    premake.w("\nMAKE := \"$(MAKE)\"")
end