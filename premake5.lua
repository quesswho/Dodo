workspace "Dodo"
	architecture "x64"
	startproject "Sandbox"
	
	outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
	
	configurations
	{
		"Debug",
		"Release"
	}
	
	group "Dependencies"
		include "Dodo/lib/glad"
	
	group "" -- Go to root level
	project "Dodo"
		kind "StaticLib"
		location "Dodo"
		language "C++"
		cppdialect "C++17"
		staticruntime "on"

		configuration "Debug"
			targetdir ("bin/Debug-%{cfg.architecture}/%{prj.name}/")
			objdir ("bin/Debug-%{cfg.architecture}/%{prj.name}/intermediates/")
		configuration "Release"
			targetdir ("bin/Release-%{cfg.architecture}/%{prj.name}/")
			objdir ("bin/Release-%{cfg.architecture}/%{prj.name}/intermediates/")
		configuration "*"

		pchheader "pch.h"
		pchsource "%{prj.name}/src/pch.cpp"

		files { 
			"%{prj.name}/src/**.h", 
			"%{prj.name}/src/**.cpp"
		}

		includedirs { 
			"%{prj.name}/src"
		}
		
		sysincludedirs {
			"%{prj.name}/lib/spdlog/include",
			"%{prj.name}/lib/glad/include"
		}

		defines
		{
			"_CRT_SECURE_NO_WARNINGS"
		}

		links {
			"glad"
		}
	
		filter "system:windows"
			systemversion "latest"
			defines {
				"DD_x64",
				"DD_API_WIN32"
			}

		filter "configurations:Debug"
			defines {
				"DD_DEBUG"
			}
			symbols "On"
	
		filter "configurations:Release"
			defines {
				"DD_RELEASE"
			}
			optimize "On"

	project "Sandbox"
		location "Sandbox"
		language "C++"
		cppdialect "C++17"
		staticruntime "On"
		
		configuration "Debug"
			targetdir ("bin/Debug-%{cfg.architecture}/%{prj.name}/")
			objdir ("bin/Debug-%{cfg.architecture}/%{prj.name}/intermediates/")
		configuration "Release"
			targetdir ("bin/Release-%{cfg.architecture}/%{prj.name}/")
			objdir ("bin/Release-%{cfg.architecture}/%{prj.name}/intermediates/")
		configuration "*"
		
		files { 
			"%{prj.name}/src/**.h",
			"%{prj.name}/src/**.cpp" 
		}

		includedirs { 
			"Dodo/src"
		}
		
		sysincludedirs {
			"Dodo/lib/spdlog/include",
			"Dodo/lib/glad/include"
		}

		links {
			"Dodo"
		}

		filter "system:windows"
			systemversion "latest"
			defines {
				"DD_x64",
				"DD_API_WIN32"
			}
		filter "configurations:Debug"
			kind "ConsoleApp"
			defines {
				"DD_DEBUG"
			}
			symbols "On"
	
		filter "configurations:Release"
			-- entrypoint "mainCRTStartup"
			-- kind "WindowedApp"
			kind "ConsoleApp"
			defines {
				"DD_RELEASE"
			}
			optimize "On"