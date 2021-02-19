@echo off

ECHO]
if "%1" equ "release" (
    ECHO ------------------------------------------
    ECHO *** BUILDING RELEASE BUILD FROM SOURCE ***
    ECHO ------------------------------------------
) else (
    ECHO ----------------------------------------
    ECHO *** BUILDING DEBUG BUILD FROM SOURCE ***
    ECHO ----------------------------------------
)

set SOURCE=win32_ray.cpp ray.cpp
set OUTPUT=ray.exe

set SHARED_FLAGS=-g -gcodeview -W -Wall -Wextra -Werror -Wno-unused-function -Wno-deprecated-declarations -Wno-unused-parameter -Wno-unused-variable -Wno-writable-strings -Wno-reorder-init-list -Wno-missing-field-initializers -Wno-missing-braces -Wno-c99-designator -msse4.1 -ferror-limit=3
set DEBUG_FLAGS=-O0 -DRAY_DEBUG=1
set RELEASE_FLAGS=-O3
set LINK_LIBRARIES=-luser32.lib -lgdi32.lib -lopengl32.lib
set INCLUDE_DIRECTORIES=-Iexternal\ -Iexternal\md\ -Igenerated\

if not exist ..\build mkdir ..\build
if not exist generated mkdir generated

REM metaprogram
del /Q generated\* 
clang metaprogram.c %INCLUDE_DIRECTORIES% %SHARED_FLAGS% -o ..\build\metaprogram.exe
..\build\metaprogram.exe

if "%1" equ "release" (
    set FLAGS=%SHARED_FLAGS% %RELEASE_FLAGS%
) else (
    set FLAGS=%SHARED_FLAGS% %DEBUG_FLAGS%
)

clang++ %SOURCE% %INCLUDE_DIRECTORIES% %FLAGS% -std=c++20 -o ..\build\%OUTPUT% %LINK_LIBRARIES%
set LAST_ERROR=%ERRORLEVEL%

popd REM build
popd REM ..
