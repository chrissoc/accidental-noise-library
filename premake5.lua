-- Accidental Noise Library
-- premake5.lua

-- Set pat to lua library and headers
-- Alternatively, specify on the command-line
local lualib="C:/Users/Joshua/Documents/Projects/lua-5.1.4/src"
local luaheader="C:/Users/Joshua/Documents/Projects/lua-5.1.4/src"

solution "ANL"
	configurations {"Debug", "Release"}
	platforms {"x32", "x64"}
	location "../build"
	
	filter "configurations:Debug"
		defines { "DEBUG" }
		flags { "Symbols" }

	filter "configurations:Release"
		defines { "NDEBUG" }
		optimize "On"
	
	filter {}
	
	if _ACTION=="gmake" then
		buildoptions "-std=c++11"
		filter "platforms:x32"
			buildoptions "-m32"
		filter "platforms:x64"
			buildoptions "-m64"
		filter {}
	end
	
	project "toluapp"
		kind "StaticLib"
		location "../build/toluapp"
		language "C"
		includedirs "ThirdParty/toluapp/include"
		includedirs(_OPTIONS["luaheader"] or luaheader)
		libdirs(_OPTIONS["lualib"] or lualib)
		files {"ThirdParty/toluapp/src/lib/*.h", "ThirdParty/toluapp/src/lib/*.c"}
	
	project "ANLLib"
		kind "StaticLib"
		location "../build/ANL"
		language "C++"
		
		files {"VM/*.h", "VM/*.cpp", "Imaging/*.h", "Imaging/*.cpp", "templates/*.h", "vectortypes.h"}

	project "Framework"
		kind "ConsoleApp"
		location "../build/Bin"
		language "C++"
		
		includedirs(_OPTIONS["luaheader"] or luaheader)
		libdirs(_OPTIONS["lualib"] or lualib)
		
		files {"Framework/*.cpp", "Bindings/*.cpp"}
		links {"ANLLib", "toluapp"}
		
		if _OPTIONS["usedynamic"] then links "lua51.dll" else links "lua" end
		
		
		