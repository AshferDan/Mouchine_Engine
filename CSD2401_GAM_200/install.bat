@echo off
setlocal
REM ===============================================
REM Windows Install and Setup Script
REM ===============================================

echo.
echo ===============================================
echo   GAM_200 Engine Installer - Windows
echo ===============================================
echo.

REM Check if CMake is available
cmake --version >nul 2>&1
if %errorlevel% neq 0 (
    echo ERROR: CMake is not installed or not in PATH
    echo Please install CMake from https://cmake.org/download/
    pause
    exit /b 1
)

REM Check if Visual Studio Build Tools are available
where cl >nul 2>&1
if %errorlevel% neq 0 (
    echo WARNING: Visual Studio Build Tools not found in PATH
    echo Make sure you have Visual Studio or Build Tools installed
    echo.
)

REM -----------------------------------------------
REM Step 1: Create Asset folder structure
REM -----------------------------------------------
echo [1/5] Setting up Assets directory structure...

if not exist Assets mkdir Assets
echo    Created: Assets\

for %%D in (Audio Prefabs Replay Scenes Scripts Sprites Tiles) do (
    if not exist "Assets\%%D" (
        mkdir "Assets\%%D"
        echo    Created: Assets\%%D\
    ) else (
        echo    Already exists: Assets\%%D\
    )
)

REM -----------------------------------------------
REM Step 2: Copy Editor_Icons
REM -----------------------------------------------
echo.
echo [2/5] Copying Editor_Icons...

if not exist "Assets\Editor_Icons" mkdir "Assets\Editor_Icons"

if exist "Assets\Editor_Icons" (
    xcopy /E /I /Y "Assets\Editor_Icons\*" "Assets\Editor_Icons\" >nul 2>&1
    echo    Editor_Icons already in place, verifying contents...
) else (
    echo    ERROR: Source Editor_Icons folder not found
    pause
    exit /b 1
)

REM Count copied icons
set ICON_COUNT=0
for %%F in ("Assets\Editor_Icons\*.png") do set /a ICON_COUNT+=1
echo    Icons found: %ICON_COUNT% PNG file(s)

REM -----------------------------------------------
REM Step 3: Set up Game folder with default main.cpp
REM -----------------------------------------------
echo.
echo [3/5] Setting up Game folder...

if not exist Game mkdir Game
echo    Created: Game\

if exist "Engine\default\main.cpp" (
    if not exist "Game\main.cpp" (
        copy "Engine\default\main.cpp" "Game\main.cpp" >nul
        echo    Copied: Engine\default\main.cpp -> Game\main.cpp
    ) else (
        echo    Already exists: Game\main.cpp (skipping)
    )
) else (
    echo    WARNING: Engine\default\main.cpp not found, skipping
)

if exist "Engine\default\main.h" (
    if not exist "Game\main.h" (
        copy "Engine\default\main.h" "Game\main.h" >nul
        echo    Copied: Engine\default\main.h -> Game\main.h
    ) else (
        echo    Already exists: Game\main.h (skipping)
    )
) else (
    echo    WARNING: Engine\default\main.h not found, skipping
)

REM -----------------------------------------------
REM Step 4: Configure Engine with CMake
REM -----------------------------------------------
echo.
echo [4/5] Configuring project with CMake...

if exist Build (
    echo    Existing Build directory found, skipping clean
) else (
    mkdir Build
)

cd Build
cmake .. -G "Visual Studio 17 2022" -A x64 -DCMAKE_POLICY_VERSION_MINIMUM=3.5
if %errorlevel% neq 0 (
    echo ERROR: CMake configuration failed
    cd ..
    pause
    exit /b 1
)
cd ..

REM -----------------------------------------------
REM Step 5: Build the Engine
REM -----------------------------------------------
echo.
echo [5/5] Building Engine with parallel compilation...

cd Build
cmake --build . --config Debug --parallel
if %errorlevel% neq 0 (
    echo ERROR: Build failed
    cd ..
    pause
    exit /b 1
)
cd ..

echo.
echo ===============================================
echo   Installation completed successfully!
echo ===============================================
echo.
echo   Assets\           - Asset directories created
echo   Assets\Editor_Icons\ - Editor icons ready
echo   Game\main.cpp     - Default entry point ready
echo   Build\            - CMake build configured
echo.
echo Press any key to exit...
pause >nul
