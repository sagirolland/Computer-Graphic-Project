@echo off

REM To compile for windows, I use clang-cl. msvc-cl (the standard visual studio compiler)
REM does not support c99 correctly, mainly designated initalizers (that I like to use)
REM so I prefer to use clang instead. To use clang with visual studio, open Visual Studio Installer
REM and modify your installation by selecting: "Individual components" -> "C++ Clang Compiler for Windows"

REM Check if the compiler exists in path and if not, set it up.
REM Update "VC_PATH" to your Visual Studio installation
REM or run the script through "x64 Native Tools Command Prompt for VS"
REM set "VC_PATH=C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC"

where /q cl
if ERRORLEVEL 1 (
  call "%VC_PATH%\Auxiliary\Build\vcvarsall.bat" x64
)

REM Figure out all relevant paths
set "SRC_ROOT=%~dp0%"

pushd "%SRC_ROOT%"

echo Building CG Skeleton

REM Build with a simple one line command
REM /Zi adds debug info generation, /Od sets the optimization to none (replace with /O2 for optimized build)
REM /I instructs the compiler to search #include files and libraries in the paths that follow, respectively
REM /Fe specifies the output executable name
REM /link specify linker flags and input library. /LIBPATH:path adds a path to search .lib files
REM /NODEFAULTLIB removes libraries that otherwise would be linked by default
clang-cl /nologo /Zi /Od /I"%SRC_ROOT%lib\include" ^
    "%SRC_ROOT%\main.c" ^
    /Fe"cg" ^
    /link /LIBPATH:"%SRC_ROOT%lib\lib-win" /NODEFAULTLIB:LIBCMT ^
    raylib.lib gdi32.lib msvcrt.lib winmm.lib user32.lib shell32.lib

popd
