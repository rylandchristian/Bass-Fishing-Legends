@echo off
REM Rebuild BassFishingLegendsEditor for Win64 from a Developer Command Prompt
REM or a normal cmd.exe. Override the engine with:  set UE_ROOT=C:\UE_5.8
setlocal EnableExtensions

set "ROOT=%~dp0.."
pushd "%ROOT%" >nul
set "PROJECT=%CD%\BassFishingLegends.uproject"

set "ENGINE="
if defined UE_ROOT if exist "%UE_ROOT%\Engine\Build\BatchFiles\Build.bat" set "ENGINE=%UE_ROOT%"
if not defined ENGINE if exist "C:\UE_5.8\Engine\Build\BatchFiles\Build.bat" set "ENGINE=C:\UE_5.8"
if not defined ENGINE if exist "%ProgramFiles%\Epic Games\UE_5.8\Engine\Build\BatchFiles\Build.bat" set "ENGINE=%ProgramFiles%\Epic Games\UE_5.8"

if not defined ENGINE (
	echo Build.bat not found. Set UE_ROOT to your UE 5.8 engine folder, e.g.:
	echo   set UE_ROOT=C:\UE_5.8
	echo   scripts\rebuild-editor.bat
	popd
	exit /b 1
)

echo Engine:  %ENGINE%
echo Project: %PROJECT%
echo Cleaning Intermediate and Binaries...
if exist Intermediate rmdir /s /q Intermediate
if exist Binaries rmdir /s /q Binaries

echo Building BassFishingLegendsEditor Win64 Development...
call "%ENGINE%\Engine\Build\BatchFiles\Build.bat" BassFishingLegendsEditor Win64 Development -Project="%PROJECT%" -WaitMutex
set "ERR=%ERRORLEVEL%"
if not "%ERR%"=="0" goto :fail

echo.
echo Build succeeded. Double-click BassFishingLegends.uproject to open the editor.
popd
exit /b 0

:fail
echo.
echo BUILD FAILED. Exit code %ERR%. Open this log and search for error C:
echo %LOCALAPPDATA%\UnrealBuildTool\Log.txt
popd
exit /b %ERR%
