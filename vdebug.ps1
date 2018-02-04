pushd .\debug

cl ..\Source\game.c ..\Source\platform_api.c ..\Source\main.c /Zi /Fe:breakout.exe /W4 /D"DEBUG_PRINT" /Od Kernel32.lib User32.lib Gdi32.lib Winmm.lib

popd
