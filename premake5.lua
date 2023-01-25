workspace "Dodo"
	architecture "x64"
	startproject "Dodeditor"
	
	outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
	
	configurations
	{
		"Debug",
		"Release"
	}
	
	group "Dependencies"
		include "Dodo/lib/glad"
		include "Dodo/lib/imgui"
		include "Dodo/lib/assimp"
	
	group "" -- Go to root level
	project "Dodo"
		kind "StaticLib"
		location "Dodo"
		language "C++"
		cppdialect "C++17"
		staticruntime "on"
		characterset "MBCS"
		
		filter "configurations:Debug"
			targetdir ("bin/Debug-%{cfg.architecture}/%{prj.name}/")
			objdir ("bin/Debug-%{cfg.architecture}/%{prj.name}/intermediates/")
			filter "configurations:Release"
			targetdir ("bin/Release-%{cfg.architecture}/%{prj.name}/")
			objdir ("bin/Release-%{cfg.architecture}/%{prj.name}/intermediates/")
		filter "*"

		pchheader "pch.h"
		pchsource "%{prj.name}/src/pch.cpp"

		files { 
			"%{prj.name}/src/**.h", 
			"%{prj.name}/src/**.cpp",
			"%{prj.name}/lib/stb_image/stb_image.h",
			"%{prj.name}/lib/stb_image/stb_image.cpp"
		}

		includedirs { 
			"%{prj.name}/src",
			"%{prj.name}/lib/stb_image/"
		}
		
		externalincludedirs {
			"%{prj.name}/lib/spdlog/include",
			"%{prj.name}/lib/glad/include",
			"%{prj.name}/lib/imgui/include",
			"%{prj.name}/lib/assimp/include"
		}

		defines
		{
			"_CRT_SECURE_NO_WARNINGS"
		}
		
		links {
			"glad",
			"opengl32.lib",
			"imgui",
			"Assimp"
		}
		
		disablewarnings { 
			"26812",
			"26451"		
		}
		
		buildoptions { 
			"/bigobj",
			"/Ob1"
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
		characterset "MBCS"
		
		filter "configurations:Debug"
			targetdir ("bin/Debug-%{cfg.architecture}/%{prj.name}/")
			objdir ("bin/Debug-%{cfg.architecture}/%{prj.name}/intermediates/")
			filter "configurations:Release"
			targetdir ("bin/Release-%{cfg.architecture}/%{prj.name}/")
			objdir ("bin/Release-%{cfg.architecture}/%{prj.name}/intermediates/")
		filter "*"
		
		files { 
			"%{prj.name}/src/**.h",
			"%{prj.name}/src/**.cpp" 
		}

		includedirs { 
			"Dodo/src"
		}
		
		externalincludedirs {
			"Dodo/lib/spdlog/include",
			"Dodo/lib/glad/include",
			"Dodo/lib/imgui/include",
			"Dodo/lib/assimp/include"
		}

		links {
			"Dodo"
		}
		
		defines
		{
			"_CRT_SECURE_NO_WARNINGS"
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
			
	project "Dodeditor"
		location "Dodeditor"
		language "C++"
		cppdialect "C++17"
		staticruntime "On"
		characterset "MBCS"
		
		filter "configurations:Debug"
			targetdir ("bin/Debug-%{cfg.architecture}/%{prj.name}/")
			objdir ("bin/Debug-%{cfg.architecture}/%{prj.name}/intermediates/")
		filter "configurations:Release"
			targetdir ("bin/Release-%{cfg.architecture}/%{prj.name}/")
			objdir ("bin/Release-%{cfg.architecture}/%{prj.name}/intermediates/")
		filter "*"
		
		disablewarnings { 
			"26812",
			"26451"		
		}
		
		files { 
			"%{prj.name}/src/**.h",
			"%{prj.name}/src/**.cpp",
			"%{prj.name}/res/**.*"
		}

		includedirs { 
			"Dodo/src"
		}
		
		externalincludedirs {
			"Dodo/lib/spdlog/include",
			"Dodo/lib/glad/include",
			"Dodo/lib/imgui/include",
			"Dodo/lib/assimp/include"
		}

		links {
			"Dodo"
		}
		
		defines
		{
			"_CRT_SECURE_NO_WARNINGS"
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