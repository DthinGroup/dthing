REM #######################################################################
REM Copyright [2013] - [2014] Yarlungsoft
REM
REM Licensed under the Apache License, Version 2.0 (the "License");
REM you may not use this file except in compliance with the License.
REM You may obtain a copy of the License at
REM
REM http://www.apache.org/licenses/LICENSE-2.0
REM
REM Unless required by applicable law or agreed to in writing, software
REM distributed under the License is distributed on an "AS IS" BASIS,
REM WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
REM See the License for the specific language governing permissions and
REM limitations under the License.
REM #######################################################################

@echo off

@setlocal

REM ####################  set compiler environment #################
if not exist env_init.bat (
    echo ##############################################
    echo Set your shell environment in env_init.bat
    echo ##############################################
)
REM call env_init.bat>nul 2>nul
call env_init.bat

set PATH=./tools;%PATH%

set CLEANBUILD=
set BINSBUILD=
set BUILDARCH=msvc
for %%i in (%*) do (
    if /i "%%i" EQU "help" goto HELP

    if /i "%%i" EQU "clean" (
        set CLEANBUILD=TRUE
    ) else if /i "%%i" EQU "bin" (
        set BINSBUILD=TRUE
    ) else if /i "%%i" EQU "lib" (
        set BINSBUILD=FALSE
    ) else if /i "%%i" EQU "arm" (
        set BUILDARCH=rvct
    ) else if /i "%%i" EQU "sim" (
        set BUILDARCH=msvc
    ) else (
        echo "Unknown parameter: %%i"
        goto HELP
    )
)

:BUILD
if "%CLEANBUILD%" EQU "TRUE" (
    gmake TOOLCHAIN=%BUILDARCH% clean
)

if "%BINSBUILD%" EQU "TRUE" (
    gmake TOOLCHAIN=%BUILDARCH% bin
) else if "%BINSBUILD%" EQU "FALSE" (
    gmake TOOLCHAIN=%BUILDARCH% lib
) else if "%CLEANBUILD%" NEQ "TRUE" (
    @echo "No parameters are input, refer to help"
    goto HELP;
)

if "%BUILDARCH%" EQU "rvct" (
  call rvct_release.bat
)

goto EXIT;

:HELP
echo ###################################################################
echo    build [[clean] ^| [bin ^| lib] ^| [arm ^| sim]]
echo        clean - clean all builds
echo        bin   - generate win32 exe file of DThing
echo        lib   - generate core library of DThing without porting layer
echo        help  - disply this info
echo.
echo        TOOLCHAIN
echo            arm: enable arm build.
echo            sim: enable MSVS build.
echo.
echo    for example:
echo        build clean - clean all previous builds
echo        build lib   - only start lib build, no clean
echo        build clean bin sim - clean first, then start bin build on sim
echo        build clean lib arm - clean first, then start lib build on arm
echo ####################################################################

:BUILD_GUIDE
echo Entering build guide mode...                                 
echo ---------------------------
echo [0] normal build
echo [1] clean build
echo ---------------------------
echo Please select build mode and press Enter for confirmation...
SET /P BUILD_MODE_ID=

if "%BUILD_MODE_ID%"=="1" (
    set CLEANBUILD=TRUE
) else (
    set CLEANBUILD=FALSE
)

echo ---------------------------
echo [0] library
echo [1] binary
echo ---------------------------
echo Please select build output and press Enter for confirmation...
SET /P BUILD_OUTPUT_ID=

if "%BUILD_OUTPUT_ID%"=="1" (
    set BINSBUILD=TRUE
) else (
    set BINSBUILD=FALSE
)

echo ---------------------------
echo [0] arm
echo [1] sim (x86)
echo ---------------------------
echo Please select build arch format and press Enter for confirmation...
SET /P BUILD_ARCH_ID=

if "%BUILD_ARCH_ID%"=="1" (
    set BUILDARCH=msvc
    XCOPY /R /Y vm\inc\x86\vm_macros.h vm\inc\
) else (
    set BUILDARCH=rvct
    XCOPY /R /Y vm\inc\arm\vm_macros.h vm\inc\
)

goto BUILD;

:EXIT

@endlocal
