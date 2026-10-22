@echo off


setlocal

set "COMPILER=gcc"

:: tested on clang and gcc

%COMPILER% xtra_python_shim.c -shared -o xtra_python_shim.dll

endlocal

