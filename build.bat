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
call env_init.bat>nul 2>nul

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
        @echo "Unknown parameter: %%i"
        goto HELP
    )
)

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

:EXIT

@endlocal
