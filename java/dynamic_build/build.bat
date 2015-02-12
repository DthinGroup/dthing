@ECHO OFF
SETLOCAL
REM  #################################################################
REM     This batch file is used to setup build enviroment, including
REM   compiling tools, scripts, etc. Before real building, it also
REM   can be called to check the environment tools.
REM  #################################################################

SET ANT_CMD=D:\tools\apache-ant-1.9.4\bin\ant.bat
SET PYTHON_CMD=C:\Python27\python.exe
SET DX_TOOL=D:\android\adt-bundle-windows-x86-20140702\sdk\build-tools\android-4.4W\dx.bat


SET SOURCE_PATH=none
SET /A SOURCE_INDEX=1

FOR %%i IN (%*) DO (
    SET /A SOURCE_INDEX+=1

    IF /i "%%i" EQU "check" (
        GOTO BUILD_CHECK

    )
    IF /i "%%i" EQU "clean" (
        GOTO BUILD_CLEAN
    )
    IF /i "%%i" EQU "help" (
        GOTO BUILD_HELP
    )
    IF /i "%%i" EQU "start" (
        GOTO BUILD_PARAMS
    )
)


:BUILD_PARAMS
FOR /L %%i IN (1, 1, %SOURCE_INDEX%) DO SHIFT
IF "%0" EQU "--source" (
    SET SOURCE_PATH=%~1
) else (
    ECHO "#Warning: Java source folder is not specified, using default path."
    SET SOURCE_PATH=%CD%\src
)
IF NOT EXIST "%SOURCE_PATH%" (
    ECHO [%SOURCE_PATH%] is not existed!
    GOTO BUILD_HELP
)
GOTO BUILD_START

:BUILD_CLEAN
DEL /Q /S "%CD%\gen\*"
GOTO END


:BUILD_CHECK
IF NOT EXIST "%PYTHON_CMD%" (
    ECHO #Error: Python path is incorrect
) else (
    %PYTHON_CMD% .\scripts\build.py --check
)
GOTO END


:BUILD_START
%PYTHON_CMD% .\scripts\build.py --start --source="%SOURCE_PATH%"
GOTO END

:BUILD_HELP
ECHO\
ECHO build.bat [check^|clean^|start[--source=path]]
ECHO   check - Check current build environment if has any problems
ECHO   start - Start build java source code into a library.
ECHO   clean - Clean build.
ECHO\
ECHO   --source=
ECHO             When calling build.bat start, the java source code path can be
ECHO           specified by parameter '--source=path', and the 'path' is java
ECHO           source code root direcotry(not include java package path).
ECHO             If source path is not specified on command line by '--source', 
ECHO           the default 'src' directory will be used as java source code root
ECHO           path which is under 'Java Build' directory, same level as this
ECHO           batch file.
ECHO\
ECHO             Here is an example for how to use '--source=':
ECHO           A java file: D:\JavaSolution\src\com\yarlungsoft\Application.java
ECHO           The package is 'com\yarlungsoft', java file is 'Application.java'
ECHO           So the root path is "D:\JavaSolution\src", below command is valid
ECHO           to start a java build:
ECHO             "build.bat start --source=D:\JavaSolution\src"


:END

ENDLOCAL
