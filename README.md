# Breakout

The old game where you try to murder innocent bricks with a ball.


## Build

! You need to remove the "TESTING\_BUILD" define, or the
ball will just bounce when it hits the bottom screen.

open with visual studio -> compile

or:

use Powershell (vbuild.ps1) to build.
The vbuild.ps1 script expects Microsoft's C compiler cl to be installed.

You can get a hold of cl by downloading Visual Studio.
After cl's downloaded, cd to the VC directory, which is located in the
main directory of the Visual Studio installation.

Inside there is vcvarsall.bat script that you can run to populate your
terminal with every environment variable needed to compile.
(I also give vcvarsall.bat the argument x64, to compile in 64 bit mode)

If you use another compiler you'll have to check vbuild.ps1 for the
arguments I pass to cl.

You'll also need an asset for the ball hit sound, or you could just comment that part out.
(the game expects the asset to be in a c-array)

## Run

Just run the .exe (found in the \Build directory) from the command line.
(or run it in visual studio)

Reset the game with CTRL + R.

There may be bugs, or things not yet implemented.
