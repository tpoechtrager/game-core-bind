@echo off
setlocal

:: Set output directory (passed in from MSBuild)
set "OUTDIR=%~1"

:: Create output directory if needed
if not exist "%OUTDIR%" mkdir "%OUTDIR%"

:: Copy lua54.dll
copy /Y "..\lib\x86_64\lua54.dll" "%OUTDIR%" >nul

:: Try to get Git build number and revision
for /f %%i in ('git rev-list --count HEAD 2^>nul') do set BUILD=%%i
if not defined BUILD set BUILD=0

for /f %%i in ('git rev-parse --short HEAD 2^>nul') do set REV=%%i
if not defined REV set REV=unknown

:: Write version.lua
(
  echo gcb.version = {
  echo   Build = %BUILD%,
  echo   GitRev = "%REV%"
  echo }
) > "%OUTDIR%\version.lua"

endlocal
