@echo off


setlocal

set "COMPILER=gcc"

:: tested on clang and gcc

%COMPILER% xtra.c -shared -DLIB_BUILD -o xtra.dll

endlocal
