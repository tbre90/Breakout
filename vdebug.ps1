﻿pushd .\debug

cl ..\Source\CPP\platform_sound.cpp ..\Source\C\platform.c ..\Source\C\game.c ..\Source\C\main.c /EHsc /Zi /Fe:breakout.exe /W4 /Od /D"_CRT_SECURE_NO_WARNINGS" /D"DEBUG_PRINT" Xaudio2.lib winmm.lib Kernel32.lib User32.lib Gdi32.lib Ole32.lib

popd
