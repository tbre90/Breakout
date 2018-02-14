pushd .\debug

cl ..\Source\platform_sound.cpp ..\Source\platform.c ..\Source\game.c ..\Source\main.c /Zi /Fe:breakout.exe /W4 /Od /D"_CRT_SECURE_NO_WARNINGS" /D"DEBUG_PRINT" Xaudio2.lib winmm.lib Kernel32.lib User32.lib Gdi32.lib

popd
