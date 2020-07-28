project "Assimp"
	kind "StaticLib"
	language "C++"
	staticruntime "on"

	targetdir ("bin/" .. outputdir .. "/")
	objdir ("bin/" .. outputdir .. "/%{prj.name}_intermediates")
	
	disablewarnings { 
	"26812",
	"26451"
	}
	
	files
	{
		"revision.h",
		"include/**.h",
		"include/**.inl",
		"code/**.*",
		"contrib/clipper/clipper.cpp",
		"contrib/clipper/clipper.hpp",
		"contrib/Open3DGC/**.*",
		"contrib/openddlparser/code/**.cpp",
		"contrib/openddlparser/include/**.h",
		"contrib/poly2tri/poly2tri/**.*",
		"contrib/unzip/**.*"
	}
	
	includedirs 
	{ 
		"include",
		".",
		"code",
		"contrib",
		"contrib/zlib",
		"contrib/irrXML",
		"contrib/unzip",
		"contrib/openddlparser/include",
		"contrib/rapidjson/include"
	}
	
	sysincludedirs
	{
		
	}
	
	links
	{
		"IrrXML",
		"zlib"
	}
	
	flags { "MultiProcessorCompile" }
	
	buildoptions { "/bigobj" }
	editAndContinue "Off"
	warnings "Extra"
	
	filter "system:windows"
			staticruntime "On"
	
	filter "configurations:Debug"
		defines
		{
			"WIN32",
			"_WINDOWS",
			"_DEBUG",
			"WIN32_LEAN_AND_MEAN",
			"UNICODE",
			"_UNICODE",
			"ASSIMP_BUILD_NO_C4D_IMPORTER",
			"MINIZ_USE_UNALIGNED_LOADS_AND_STORES=0",
			"ASSIMP_IMPORTER_GLTF_USE_OPEN3DGC=1",
			"_SCL_SECURE_NO_WARNINGS",
			"_CRT_SECURE_NO_WARNINGS",
			"OPENDDLPARSER_BUILD",
			"CMAKE_INTDIR=\"Debug\""
		}
		
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		defines
		{
			"WIN32",
			"_WINDOWS",
			"NDEBUG",
			"WIN32_LEAN_AND_MEAN",
			"UNICODE",
			"_UNICODE",
			"ASSIMP_BUILD_NO_C4D_IMPORTER",
			"MINIZ_USE_UNALIGNED_LOADS_AND_STORES=0",
			"ASSIMP_IMPORTER_GLTF_USE_OPEN3DGC=1",
			"_SCL_SECURE_NO_WARNINGS",
			"_CRT_SECURE_NO_WARNINGS",
			"OPENDDLPARSER_BUILD",
			"CMAKE_INTDIR=\"Release\""
		}
		runtime "Release"
		optimize "on"
		
		
group "Dependencies/AssimpDependencies"
project "IrrXML"
	kind "StaticLib"
	language "C++"
	staticruntime "on"
	
	targetdir ("bin/" .. outputdir .. "/")
	objdir ("bin/" .. outputdir .. "/%{prj.name}_intermediates")
	
	files
	{
		"contrib/irrXML/*.h",
		"contrib/irrXML/*.cpp"
	}
	
	flags { "MultiProcessorCompile" }
	buildoptions { "/bigobj" }
	editAndContinue "Off"
	
	filter "configurations:Debug"
		defines
		{
			"WIN32",
			"_WINDOWS",
			"_DEBUG",
			"WIN32_LEAN_AND_MEAN",
			"UNICODE",
			"_UNICODE",
			"ASSIMP_BUILD_NO_C4D_IMPORTER",
			"MINIZ_USE_UNALIGNED_LOADS_AND_STORES=0",
			"ASSIMP_IMPORTER_GLTF_USE_OPEN3DGC=1",
			"_SCL_SECURE_NO_WARNINGS",
			"_CRT_SECURE_NO_WARNINGS",
			"OPENDDLPARSER_BUILD",
			"CMAKE_INTDIR=\"Debug\""
		}
		
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		defines
		{
			"WIN32",
			"_WINDOWS",
			"NDEBUG",
			"WIN32_LEAN_AND_MEAN",
			"UNICODE",
			"_UNICODE",
			"ASSIMP_BUILD_NO_C4D_IMPORTER",
			"MINIZ_USE_UNALIGNED_LOADS_AND_STORES=0",
			"ASSIMP_IMPORTER_GLTF_USE_OPEN3DGC=1",
			"_SCL_SECURE_NO_WARNINGS",
			"_CRT_SECURE_NO_WARNINGS",
			"OPENDDLPARSER_BUILD",
			"CMAKE_INTDIR=\"Release\""
		}
		runtime "Release"
		optimize "on"
project "zlib"
	kind "StaticLib"
	language "C"
	staticruntime "on"
	
	targetdir ("bin/" .. outputdir .. "/")
	objdir ("bin/" .. outputdir .. "/%{prj.name}_intermediates")
	
	files
	{
		"contrib/zlib/*.h",
		"contrib/zlib/*.c"
	}
	
	flags { "MultiProcessorCompile" }
	buildoptions { "/bigobj" }
	editAndContinue "Off"
	
	filter "configurations:Debug"
		defines
		{
			"WIN32",
			"_WINDOWS",
			"WIN32_LEAN_AND_MEAN",
			"UNICODE",
			"_UNICODE",
			"NO_FSEEKO",
			"_CRT_SECURE_NO_DEPRECATE",
			"_CRT_NONSTDC_NO_DEPRECATE",
			"CMAKE_INTDIR=\"Debug\""
		}
		
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		defines
		{
			"WIN32",
			"_WINDOWS",
			"NDEBUG",
			"WIN32_LEAN_AND_MEAN",
			"UNICODE",
			"_UNICODE",
			"NO_FSEEKO",
			"_CRT_SECURE_NO_DEPRECATE",
			"_CRT_NONSTDC_NO_DEPRECATE",
			"CMAKE_INTDIR=\"Release\""
		}
		runtime "Release"
		optimize "on"