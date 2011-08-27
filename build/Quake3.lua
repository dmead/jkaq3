-- Quake3 Solution
solution("jamp")
	configurations { "Debug", "Release" }
	location (_ACTION)
 
	-- [start] Settings that are true for all projects
	if isVisualStudio then
		defines { "WIN32", "_CRT_SECURE_NO_DEPRECATE" }
	end

	configuration "Debug"
		if isVisualStudio then
			defines { "_DEBUG" }
		else
			defines { "DEBUG" }
		end
		flags { "Symbols" }

	configuration "Release"
		defines { "NDEBUG" }
		flags { "Optimize" }

	-- [end] Settings that are true for all projects

-- Engine/Dedicated Server Executable
for isEngine = 0, 1, 1 do
	if isEngine == 1 then
		engineName = "engine"
	else -- isDedicated
		engineName = "dedicatedServer"
	end
	project( engineName )
		if isEngine == 1 then
			uuid "ADDB20B6-BCA6-F14A-8BB8-F7F3504279E3"
			kind "WindowedApp"
		else -- isDedicated
			uuid "07CF10E7-92C6-5D47-86CB-8E900CFAB7AD"
			kind "ConsoleApp"
		end
		language "C"
		targetdir( binaryDir )
		includedirs { sdlPath, curlPath, oalPath, speexPath, zlibPath, jpegPath, madPath }
		if isWindows then
			libdirs { quakePath .. "/libs/win32" }
		end
		defines { "BOTLIB", "STANDALONE", "USE_ICON", "USE_CURL", "USE_CURL_DLOPEN", "USE_OPENAL", "USE_OPENAL_DLOPEN", "USE_VOIP", "HAVE_CONFIG_H", "USE_LOCAL_HEADERS", "USE_INTERNAL_JPEG", "_JK2MP"  }
		if isVisualStudio then
			links { "ws2_32", "winmm", "user32", "advapi32", "wsock32", "msvcrt", "psapi" }
		end
		if isEngine == 1 then
			links { "SDLmain", "SDL", "dxguid", "OpenGL32", "libmad" }
			files { quakePath .. "/renderer/*.h", quakePath .. "/renderer/*.c" }
			files { quakePath .. "/client/*.h", quakePath .. "/client/*.c" }
			files { quakePath .. "/sdl/*.h", quakePath .. "/sdl/*.c" }
			files { quakePath .. "/jpeg-8c/*.h", quakePath .. "/jpeg-8c/*.c" }			
			files { quakePath .. "/libspeex/*.h", quakePath .. "/libspeex/*.c" }
			excludes { quakePath .. "/client/libmumblelink.*" } -- fuck mumble
			excludes { quakePath .. "/renderer/tr_subs.c" } -- modular renderer only
			files { quakePath .. "/sys/con_log.c", quakePath .. "/sys/con_passive.c", quakePath .. "/sys/sys_loadlib.h", quakePath .. "/sys/sys_local.h", quakePath .. "/sys/sys_main.c", quakePath .. "/sys/sys_win32.c" }
		else -- isDedicated
			defines { "DEDICATED"  }
			files { quakePath .. "/null/null_client.c", quakePath .. "/null/null_input.c", quakePath .. "/null/null_snddma.c"  }
			files { quakePath .. "/sys/con_log.c", quakePath .. "/sys/con_win32.c", quakePath .. "/sys/sys_loadlib.h", quakePath .. "/sys/sys_local.h", quakePath .. "/sys/sys_main.c", quakePath .. "/sys/sys_win32.c" }
		end
		
		files { quakePath .. "/botlib/*.h", quakePath .. "/botlib/*.c" }
		files { quakePath .. "/qcommon/*.h", quakePath .. "/qcommon/*.c" }
		files { quakePath .. "/server/*.h", quakePath .. "/server/*.c" }
		files { quakePath .. "/zlib/*.h", quakePath .. "/zlib/*.c" }
		
		excludes { quakePath .. "/server/sv_rankings.c" }

		-- Should probably be fixed to work for mingw too
		if isVisualStudio then
			files { quakePath .. "/sys/win_resource.*" }
		else -- isVisualStudio
			excludes { quakePath .. "/sys/win_resource.*" }
		end

		objdir( objectDir .. "/JAMP/" .. engineName )
end

--[[
	-- Game Shared Library
	project("game")
		uuid "290EF3B3-EF14-984A-863B-B4C19C137F7F"
		kind "SharedLib"
		language "C"
		targetdir( binaryDir )
		targetname( "jampgamex86" )
		defines { "QAGAME", "_JK2MP" }
		files { quakePath .. "/game/*.h", quakePath .. "/game/*.c", quakePath .. "/qcommon/q_*.h", quakePath .. "/qcommon/q_*.c" }
		excludes { quakePath .. "/game/bg_lib.*" } -- skip this shit
		objdir( objectDir .. "/JAMP/Game" )

	-- CGame Shared Library
	project("cgame")
		uuid "ACF29ABA-846B-B449-B6D9-F6C3C3131622"
		kind "SharedLib"
		language "C"
		targetdir( binaryDir )
		targetname( "cgamex86" )
		defines { "CGAME", "_JK2MP" }
		files { quakePath .. "/cgame/*.h", quakePath .. "/cgame/*.c", quakePath .. "/qcommon/q_*.h", quakePath .. "/qcommon/q_*.c" }
		files { quakePath .. "/game/bg_*.h", quakePath .. "/game/bg_*.c", quakePath .. "/ui/ui_shared.*" }
		objdir( objectDir .. "/JAMP/CGame" )
	
--]]
		
	-- UI Shared Library
	project "ui"
		uuid "C41A4BD4-F010-8243-811B-8806559D9B90"
		kind "SharedLib"
		language "C"
		targetdir( binaryDir )
		targetname( "uix86" )
		defines { "UI_EXPORTS", "_JK2MP" }
		--libdirs { libDirectories }
		--includedirs { librariesPath .. "/Include" }
		--links { "ogShared" }
		files { quakePath .. "/ui/*.h", quakePath .. "/ui/*.c", quakePath .. "/qcommon/q_*.h", quakePath .. "/qcommon/q_*.c" }
		files { quakePath .. "/game/bg_public.h", quakePath .. "/game/bg_local.h", quakePath .. "/game/bg_misc.c", quakePath .. "/game/bg_saga.*", quakePath .. "/game/bg_vehicleLoad.c", quakePath .. "/game/bg_weapons.*", quakePath .. "/game/bg_straph.h" }
		objdir( objectDir .. "/JAMP/UI" )


