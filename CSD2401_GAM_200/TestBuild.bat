@echo off
REM Build C# Scripts for Game Engine
REM This script compiles all C# files in the Scripts folder into GameScripts.dll

echo ======================================
echo   C# Script Compilation
echo ======================================
echo.

REM Check if Scripts directory exists
if not exist "Scripts" (
    echo ERROR: Scripts directory not found!
    echo Creating Scripts directory...
    mkdir Scripts
    echo Please add your .cs files to the Scripts folder.
    pause
    exit /b 1
)

REM Check if any .cs files exist
dir /b Scripts\*.cs >nul 2>&1
if errorlevel 1 (
    echo WARNING: No .cs files found in Scripts directory!
    echo Please add your C# script files to Scripts folder.
    pause
    exit /b 1
)

REM Try to find Mono compiler (mcs)
set MCS_PATH=
if exist "C:\Program Files\Mono\bin\mcs.exe" (
    set MCS_PATH="C:\Program Files\Mono\bin\mcs.exe"
    echo Found Mono compiler: C:\Program Files\Mono\bin\mcs.exe
) else if exist "Extern\Mono\bin\mcs.exe" (
    set MCS_PATH="Extern\Mono\bin\mcs.exe"
    echo Found Mono compiler: Extern\Mono\bin\mcs.exe
) else (
    REM Try to use mcs from PATH
    where mcs >nul 2>&1
    if not errorlevel 1 (
        set MCS_PATH=mcs
        echo Found Mono compiler in PATH
    ) else (
        echo ERROR: Mono compiler (mcs.exe) not found!
        echo Please install Mono from: https://www.mono-project.com/download/stable/
        echo Or ensure mcs.exe is in your PATH
        pause
        exit /b 1
    )
)

echo.
echo Compiling C# scripts...
echo Input: Scripts\*.cs
echo Output: Build\Scripts\GameScripts.dll
echo.

REM Create Build/Scripts directory if it doesn't exist
if not exist "Build\Scripts" mkdir "Build\Scripts"

REM Compile all C# files
%MCS_PATH% -target:library -out:Build\Scripts\GameScripts.dll Scripts\*.cs

if errorlevel 1 (
    echo.
    echo ======================================
    echo   COMPILATION FAILED!
    echo ======================================
    echo Please check the error messages above.
    pause
    exit /b 1
)

REM Also copy to Debug and Release folders if they exist
if exist "Build\Debug" (
    if not exist "Build\Debug\Scripts" mkdir "Build\Debug\Scripts"
    copy /Y "Build\Scripts\GameScripts.dll" "Build\Debug\Scripts\" >nul
    echo Copied to Build\Debug\Scripts\
)

if exist "Build\Release" (
    if not exist "Build\Release\Scripts" mkdir "Build\Release\Scripts"
    copy /Y "Build\Scripts\GameScripts.dll" "Build\Release\Scripts\" >nul
    echo Copied to Build\Release\Scripts\
)

echo.
echo ======================================
echo   COMPILATION SUCCESSFUL!
echo ======================================
echo GameScripts.dll created successfully.
echo You can now run your game.
echo.
echo TIP: You can run this script anytime to recompile your C# scripts
echo      without rebuilding the entire C++ project!
echo.
pause