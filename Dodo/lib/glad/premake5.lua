project "glad"
	kind "StaticLib"
	language "C"
	
	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin/" .. outputdir .. "/%{prj.name}/intermediates")
	
	files
	{
		"include/**.h",
		"src/*.c"
	}
	
	includedirs 
	{ 
		"include"
	}
	
	filter "system:windows"
			staticruntime "On"
	
	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		runtime "Release"
		optimize "on"