-- Prevent from script error when no action is given
if not _ACTION then
	printf "Error: No action defined!"
	return
end

-- Check for supported OS and action
if os.is( "windows" ) then
	isWindows = true
	if string.startswith(_ACTION, "vs") then
		isVisualStudio = true
	else
		printf "Warning: Not tested for this action yet!"
	end

elseif os.is( "linux" ) then
	isLinux = true
--	printf "Warning: Untested, probably needs adaption!"
	oal_soft_have = { ALSA=true, OSS=true }
elseif os.is( "macosx" ) then
	isMac = true
	printf "Warning: Untested, probably needs adaption!"
else
	printf "Error: Your OS is not supported yet"
	return
end

-- Some Paths
rootPath = ".."

objectDir		= "out/obj"
libDir			= "out"
binaryDir		= rootPath .. "/Binaries"
quakePath		= rootPath .. "/code"

-- Thirdparty Paths
--thirdPartyPath	= rootPath .. "/Thirdparty"
zlibPath		= quakePath .. "/zlib"
--pngPath			= thirdPartyPath .. "/libpng"
jpegPath		= quakePath .. "/jpeg-8c"
madPath		= quakePath .. "/mad"
--oggPath			= thirdPartyPath .. "/ogg"
--vorbisPath		= thirdPartyPath .. "/vorbis"
oalPath			= quakePath .. "/AL"
sdlPath			= quakePath .. "/SDL12/include"
speexPath	= quakePath .. "/libspeex/include"
curlPath		= quakePath .. "/libcurl"

-- The Solution
dofile "Quake3.lua"