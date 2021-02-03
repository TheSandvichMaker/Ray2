@echo off

set SOURCE=..\src\win32_ray.c ..\src\ray.c
set OUTPUT=ray.exe

pushd ..

if not exist build mkdir build
pushd build

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

set SHARED_FLAGS=-g -gcodeview -W -Wall -Wextra -Werror -Wno-unused-function -Wno-deprecated-declarations -Wno-unused-parameter -Wno-unused-variable -Wno-writable-strings
set DEBUG_FLAGS=-O0 -DDEBUG_BUILD
set RELEASE_FLAGS=-O3
set LINK_LIBRARIES=-luser32.lib -lgdi32.lib -lopengl32.lib

if "%1" equ "release" (
    set FLAGS=%SHARED_FLAGS% %RELEASE_FLAGS%
) else (
    set FLAGS=%SHARED_FLAGS% %DEBUG_FLAGS%
)

clang %SOURCE% %FLAGS% -o %OUTPUT% %LINK_LIBRARIES%
set LAST_ERROR=%ERRORLEVEL%

popd REM build
popd REM ..
