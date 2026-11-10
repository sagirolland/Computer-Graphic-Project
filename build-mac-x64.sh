#!/bin/bash

# Figure out all the paths
ME="$(readlink -f "$0")"
SRC_ROOT="$(dirname "$ME")"

# 1>/dev/null means to ignore the standard output when running the command (to reduce noise)
pushd "$SRC_ROOT" 1>/dev/null

echo Building CG Skeleton

# Build with a simple one line command
### Remove -O2 if you want to debug!!

# -Wl,-rpath,'$ORIGIN' instructs the dynamic linker to search libraries in the executable's directory so you don't need to install those libraries (specificly raylib) system-wide.
# -I and -L instructs the compiler to search #include files and libraries in the paths that follow, respectively
# -l instructs the linker to link with the library that follows
# -ggdb adds debug info generation.
# -Wall enables all warnings, -Wno-<warning> disables specfic warnings
# -std=c99 sets the C standard to C99 (needed for designated initializer)
# -O2 adds compiler optimizations, for debugging it's better to remove it as it can change the order of execution.
# -o specifies the output executable name
clang "$SRC_ROOT/main.c" \
    -Wl,-rpath,'$ORIGIN' \
    -I"$SRC_ROOT/lib/include" -L"$SRC_ROOT/lib/lib-mac-x64" \
    -lraylib -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo -framework QuartzCore \
    -ggdb -Wall -Wno-missing-braces -Wno-unused-function \
    -std=c99 \
    -O2 \
    -o cg

popd 1>/dev/null