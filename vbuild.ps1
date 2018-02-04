pushd .\build

cl ..\Source\platform_api.c ..\Source\game.c ..\Source\main.c /Fe:breakout.exe /W4 /Od /D"DEBUG_PRINT" winmm.lib Kernel32.lib User32.lib Gdi32.lib

popd

