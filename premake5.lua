workspace "Dodo"
	architecture "x64"
	startproject "Sandbox"
	
	makesettings [[
		CC = gcc
	]]
	--toolset "gcc" -- Use gnu compiler collection
	
	outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
	
	configurations
	{
		"Debug",
		"Release"
	}

	group "" -- Go to root level
	project "Dodo"
		kind "StaticLib"
		location "Dodo"
		language "C++"
		cppdialect "gnu++17"
		staticruntime "on"

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
			"%{prj.name}/src"
		}
		
		sysincludedirs {
		}

		defines
		{
			"_CRT_SECURE_NO_WARNINGS"
		}

		links {
		}
	
		filter "system:windows"
			systemversion "latest"
			defines {
				"DD_x64"
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
		kind "ConsoleApp"
		location "Sandbox"
		language "C++"
		cppdialect "gnu++17"
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
		}

		links { 
			"Dodo"
		}

		filter "system:windows"
			systemversion "latest"
			defines {
				"DD_x64"
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