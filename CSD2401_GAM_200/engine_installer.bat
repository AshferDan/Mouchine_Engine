@echo off
setlocal EnableDelayedExpansion

REM ===============================================
REM Windows Install and Setup Script
REM ===============================================

REM Resolve the directory this script lives in (no trailing backslash)
set "SOURCE_DIR=%~dp0"
set "SOURCE_DIR=%SOURCE_DIR:~0,-1%"

echo.
echo ===============================================
echo   GAM_200 Engine Installer - Windows
echo ===============================================
echo.

REM -----------------------------------------------
REM Select installation directory
REM -----------------------------------------------
echo Selecting installation folder...
echo Default: %SOURCE_DIR%
echo.

REM Use PowerShell FolderBrowserDialog; SOURCE_DIR passed via env var so backslashes are safe
for /f "usebackq delims=" %%i in (`powershell -NoProfile -Command ^
    "& { Add-Type -AssemblyName System.Windows.Forms; $d = New-Object System.Windows.Forms.FolderBrowserDialog; $d.Description = 'Select folder to install GAM_200 Engine'; $d.ShowNewFolderButton = $true; $d.SelectedPath = $Env:SOURCE_DIR; if ($d.ShowDialog() -eq 'OK') { $d.SelectedPath } else { $Env:SOURCE_DIR } }"`) do (
    set "INSTALL_DIR=%%i"
)

if not defined INSTALL_DIR set "INSTALL_DIR=%SOURCE_DIR%"

echo.
echo Install location: %INSTALL_DIR%
echo.

REM If the user picked a different folder, copy the engine distribution there first
if /i not "%INSTALL_DIR%" == "%SOURCE_DIR%" (
    echo Copying engine files to selected folder...

    REM Engine source directories (Scripts = C# scripting project)
    for %%D in (Engine Extern Shaders cmake Logs mono Scripts) do (
        if exist "%SOURCE_DIR%\%%D" (
            robocopy "%SOURCE_DIR%\%%D" "%INSTALL_DIR%\%%D" /E /NFL /NDL /NJH /NJS /NC /NS >nul
            echo    Copied: %%D\
        )
    )

    REM Loose root files
    for %%F in (CMakeLists.txt mono-2.0-sgen.dll MonoPosixHelper.dll script_build.bat script_build.sh script_clean.bat script_clean.sh script_run.bat script_run.sh) do (
        if exist "%SOURCE_DIR%\%%F" (
            copy /Y "%SOURCE_DIR%\%%F" "%INSTALL_DIR%\%%F" >nul
            echo    Copied: %%F
        )
    )

    echo    Engine files copied.
    echo.
)

REM All subsequent steps run inside the chosen install directory
cd /d "%INSTALL_DIR%"

REM -----------------------------------------------
REM Pre-flight checks
REM -----------------------------------------------
cmake --version >nul 2>&1
if %errorlevel% neq 0 (
    echo ERROR: CMake is not installed or not in PATH
    echo Please install CMake from https://cmake.org/download/
    pause
    exit /b 1
)

where cl >nul 2>&1
if %errorlevel% neq 0 (
    echo WARNING: Visual Studio Build Tools not found in PATH
    echo Make sure you have Visual Studio or Build Tools installed
    echo.
)

REM -----------------------------------------------
REM Step 1: Create Asset folder structure
REM -----------------------------------------------
echo [1/6] Setting up Assets directory structure...

if not exist Assets mkdir Assets
echo    Assets\ ready

for %%D in (Audio Prefabs Replay Scenes Scripts Sprites Tiles) do (
    if not exist "Assets\%%D" (
        mkdir "Assets\%%D"
        echo    Created: Assets\%%D\
    ) else (
        echo    Already exists: Assets\%%D\
    )
)

REM Copy GameAPI.cs into Assets\Scripts (skip if installing in-place to avoid self-copy zeroing the file)
if /i not "%INSTALL_DIR%" == "%SOURCE_DIR%" (
    if exist "%SOURCE_DIR%\Assets\Scripts\GameAPI.cs" (
        copy /Y "%SOURCE_DIR%\Assets\Scripts\GameAPI.cs" "Assets\Scripts\GameAPI.cs" >nul
        echo    Copied: GameAPI.cs -^> Assets\Scripts\GameAPI.cs
    ) else (
        echo    WARNING: GameAPI.cs not found at %SOURCE_DIR%\Assets\Scripts\, skipping
    )
) else (
    echo    GameAPI.cs already in place
)

REM -----------------------------------------------
REM Step 2: Copy Editor_Icons
REM -----------------------------------------------
echo.
echo [2/6] Copying Editor_Icons...

if not exist "Assets\Editor_Icons" mkdir "Assets\Editor_Icons"

if exist "Logs\default\Editor_Icons" (
    xcopy /E /I /Y "Logs\default\Editor_Icons\*" "Assets\Editor_Icons\" >nul 2>&1
    echo    Copied Editor_Icons from Logs\default\Editor_Icons
) else (
    echo    ERROR: Logs\default\Editor_Icons not found
    pause
    exit /b 1
)

set ICON_COUNT=0
for %%F in ("Assets\Editor_Icons\*.png") do set /a ICON_COUNT+=1
echo    Icons found: %ICON_COUNT% PNG file(s)

REM -----------------------------------------------
REM Step 3: Set up Game folder with default main.cpp / main.h
REM -----------------------------------------------
echo.
echo [3/6] Setting up Game folder...

if not exist Game mkdir Game
echo    Game\ directory ready

REM Always copy — show the error if the source is missing (no >nul suppression)
if exist "Logs\default\main.cpp" (
    copy /Y "Logs\default\main.cpp" "Game\main.cpp"
    if %errorlevel% neq 0 (
        echo    ERROR: Failed to copy main.cpp
        pause
        exit /b 1
    )
    echo    Copied: Logs\default\main.cpp -^> Game\main.cpp
) else (
    echo    ERROR: Logs\default\main.cpp not found ^(checked in %INSTALL_DIR%\Logs\default\^)
    pause
    exit /b 1
)

if exist "Logs\default\main.h" (
    copy /Y "Logs\default\main.h" "Game\main.h"
    if %errorlevel% neq 0 (
        echo    ERROR: Failed to copy main.h
        pause
        exit /b 1
    )
    echo    Copied: Logs\default\main.h -^> Game\main.h
) else (
    echo    ERROR: Logs\default\main.h not found ^(checked in %INSTALL_DIR%\Logs\default\^)
    pause
    exit /b 1
)

REM -----------------------------------------------
REM Step 4: Set up Scripts folder (C# scripting project)
REM -----------------------------------------------
echo.
echo [4/6] Setting up Scripts folder...

if exist Scripts (
    echo    Scripts\ already exists
) else (
    if exist "%SOURCE_DIR%\Scripts" (
        robocopy "%SOURCE_DIR%\Scripts" "Scripts" /E /NFL /NDL /NJH /NJS /NC /NS >nul
        echo    Copied Scripts from %SOURCE_DIR%\Scripts
    ) else (
        echo    WARNING: Scripts folder not found at %SOURCE_DIR%\Scripts, skipping
    )
)

REM -----------------------------------------------
REM Step 5: Configure Engine with CMake
REM -----------------------------------------------
echo.
echo [5/6] Configuring project with CMake...

if not exist Build mkdir Build

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
REM Step 6: Build the Engine
REM -----------------------------------------------
echo.
echo [6/6] Building Engine with parallel compilation...

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
echo   Location:            %INSTALL_DIR%
echo   Assets\              - Asset directories created
echo   Assets\Editor_Icons\ - Editor icons ready
echo   Game\main.cpp        - Default entry point ready
echo   Game\main.h          - Default header ready
echo   Scripts\             - C# scripting project ready
echo   Build\               - CMake build configured
echo.
echo Press any key to exit...
pause >nul
