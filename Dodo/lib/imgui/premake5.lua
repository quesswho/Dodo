project "imgui"
	kind "StaticLib"
	language "C"
	
	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin/" .. outputdir .. "/%{prj.name}/intermediates")
	
	disablewarnings { 
		"26812",
		"6011"
	}
	
	files
	{
		"include/*.h",
		"src/*.cpp"
	}
	
	includedirs 
	{ 
		"include"
	}
	
	sysincludedirs
	{
		"../glad/include"
	}
	
	filter "system:windows"
			staticruntime "On"
	
	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		runtime "Release"
		optimize "on"